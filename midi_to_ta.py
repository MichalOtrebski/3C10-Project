import sys
from collections import defaultdict
import mido

# ---------- Config ----------
DEFAULT_BPM = 150          # used if MIDI has no tempo events
TICKS_PER_QUARTER_FALLBACK = 480

# Quantize to 1/16 notes:
# 1 quarter note = 4 sixteenths
SIXTEENTHS_PER_QUARTER = 4

# For polyphony within one track: choose how to collapse to one note
# Options: "highest", "lowest", "last"
POLY_MODE = "highest"

# ---------- Helpers ----------
def tempo_us_to_bpm(tempo_us_per_beat: int) -> float:
    return 60_000_000.0 / tempo_us_per_beat

def clamp_midi_note(n: int) -> int:
    # Your TA_Event uses int8_t: MIDI 0..127 fits fine, keep -1 for rest
    return max(0, min(127, n))

def pick_note(active_notes, mode: str):
    if not active_notes:
        return None
    if mode == "highest":
        return max(active_notes)
    if mode == "lowest":
        return min(active_notes)
    # "last": active_notes is a set, so track separately if you need true last.
    return max(active_notes)

def ticks_to_sixteenth_ticks(delta_ticks: int, ticks_per_beat: int) -> int:
    # Convert MIDI ticks to "sixteenth ticks" (1 unit = 1/16 note)
    # delta_16 = delta_ticks * (16th per beat) / ticks_per_beat
    # 16th per beat = 4 (since beat=quarter)
    return int(round(delta_ticks * SIXTEENTHS_PER_QUARTER / ticks_per_beat))

def emit_events_from_note_timeline(timeline_16):
    """
    timeline_16: list of (note_or_None, dur16) where note_or_None is a MIDI note or None for rest
    Returns TA_Event list: (note, len16) where note=-1 for rest
    """
    out = []
    for note, dur in timeline_16:
        if dur <= 0:
            continue
        out.append((note if note is not None else -1, dur))
    # optional: merge adjacent identical notes/rests
    merged = []
    for n, d in out:
        if merged and merged[-1][0] == n:
            merged[-1] = (n, merged[-1][1] + d)
        else:
            merged.append((n, d))
    return merged

def track_to_monophonic_events(mid: mido.MidiFile, track_index: int, channel_filter=None):
    """
    Builds a monophonic note timeline from one MIDI track.
    Returns list of (note_or_None, dur16) quantized to 1/16.
    """
    ticks_per_beat = getattr(mid, "ticks_per_beat", None) or TICKS_PER_QUARTER_FALLBACK

    # Determine tempo map (use the first tempo we see; good enough for most game MIDIs)
    tempo_us = 500000  # default 120 BPM
    for t in mid.tracks:
        for msg in t:
            if msg.type == "set_tempo":
                tempo_us = msg.tempo
                break
        # don't break outer; a later track might contain tempo too (rare)
    bpm = tempo_us_to_bpm(tempo_us) if tempo_us else DEFAULT_BPM

    tr = mid.tracks[track_index]

    active = set()
    current_note = None

    timeline = []  # list of (note_or_None, dur16)
    acc16 = 0

    def flush(dur16):
        nonlocal current_note
        if dur16 <= 0:
            return
        timeline.append((current_note, dur16))

    for msg in tr:
        delta16 = ticks_to_sixteenth_ticks(msg.time, ticks_per_beat)
        if delta16:
            flush(delta16)
            acc16 += delta16

        if msg.type in ("note_on", "note_off"):
            if channel_filter is not None and getattr(msg, "channel", None) != channel_filter:
                continue

            note = msg.note
            is_on = (msg.type == "note_on" and msg.velocity > 0)
            is_off = (msg.type == "note_off") or (msg.type == "note_on" and msg.velocity == 0)

            if is_on:
                active.add(note)
            elif is_off:
                active.discard(note)

            picked = pick_note(active, POLY_MODE)
            current_note = clamp_midi_note(picked) if picked is not None else None

    return bpm, timeline

def print_c_array(name: str, events):
    print(f"static const TA_Event {name}[] = {{")
    line = "    "
    for (note, dur) in events:
        token = f"{{ {note}, {dur} }}, "
        if len(line) + len(token) > 100:
            print(line)
            line = "    " + token
        else:
            line += token
    if line.strip():
        print(line)
    print("};")
    print(f"static const size_t {name}_len = sizeof({name}) / sizeof({name}[0]);")
    print()

def main():
    if len(sys.argv) < 2:
        print("Usage: python midi_to_ta.py <file.mid> [track_indices...]")
        print("Example: python midi_to_ta.py song.mid 0 1 2 3")
        sys.exit(1)

    path = sys.argv[1]
    mid = mido.MidiFile(path)

    if len(sys.argv) == 2:
        print("Tracks in MIDI:")
        for i, tr in enumerate(mid.tracks):
            name = ""
            for msg in tr:
                if msg.type == "track_name":
                    name = msg.name
                    break
            print(f"  {i}: {name} (len={len(tr)})")
        print("\nRe-run with track indices, e.g.: python midi_to_ta.py file.mid 0 1 2 3")
        sys.exit(0)

    track_idxs = [int(x) for x in sys.argv[2:]]
    for idx_i, ti in enumerate(track_idxs):
        bpm, timeline = track_to_monophonic_events(mid, ti)
        events = emit_events_from_note_timeline(timeline)
        arr_name = ["g_melody", "g_harmony", "g_bass", "g_noise"][idx_i] if idx_i < 4 else f"g_track{idx_i}"
        print(f"// Source MIDI track {ti}, approx BPM ~ {bpm:.1f}")
        print_c_array(arr_name, events)

if __name__ == "__main__":
    main()