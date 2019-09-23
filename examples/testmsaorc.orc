
.function audio_add_s16
.dest 2 d1 short
.source 2 s1 short
.source 2 s2 short

addssw d1, s1, s2


.function audio_add_u16
.dest 2 d1
.source 2 s1
.source 2 s2

addusw d1, s1, s2