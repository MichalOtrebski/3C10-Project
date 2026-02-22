import mido
mid = mido.MidiFile("tetris.mid")
print("tpb", mid.ticks_per_beat)
for t in mid.tracks:
    for msg in t:
        if msg.type == "set_tempo":
            print("tempo_us_per_beat", msg.tempo)
            raise SystemExit
print("no tempo event found")