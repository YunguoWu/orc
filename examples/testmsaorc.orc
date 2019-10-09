
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


.function orc_addb
.dest 1 d1
.source 1 s1
.source 1 s2

addb d1, s1, s2


.function orc_addssb
.dest 1 d1
.source 1 s1
.source 1 s2

addssb d1, s1, s2


.function orc_addusb
.dest 1 d1
.source 1 s1
.source 1 s2

addusb d1, s1, s2


.function orc_addw
.dest 2 d1
.source 2 s1
.source 2 s2

addw d1, s1, s2


.function orc_addssw
.dest 2 d1
.source 2 s1
.source 2 s2

addssw d1, s1, s2


.function orc_addusw
.dest 2 d1
.source 2 s1
.source 2 s2

addusw d1, s1, s2


.function orc_addl
.dest 4 d1
.source 4 s1
.source 4 s2

addl d1, s1, s2


.function orc_addssl
.dest 4 d1
.source 4 s1
.source 4 s2

addssl d1, s1, s2


.function orc_addusl
.dest 4 d1
.source 4 s1
.source 4 s2

addusl d1, s1, s2


.function orc_addq
.dest 8 d1
.source 8 s1
.source 8 s2

addq d1, s1, s2


.function orc_add_f32
.dest 4 d1 float
.source 4 s1 float

addf d1, d1, s1


.function orc_add_f64
.dest 8 d1 double
.source 8 s1 double

addd d1, d1, s1

.function orc_subb
.dest 1 d1
.source 1 s1
.source 1 s2

subb d1, s1, s2


.function orc_subssb
.dest 1 d1
.source 1 s1
.source 1 s2

subssb d1, s1, s2


.function orc_subusb
.dest 1 d1
.source 1 s1
.source 1 s2

subusb d1, s1, s2


.function orc_subw
.dest 2 d1
.source 2 s1
.source 2 s2

subw d1, s1, s2


.function orc_subssw
.dest 2 d1
.source 2 s1
.source 2 s2

subssw d1, s1, s2


.function orc_subusw
.dest 2 d1
.source 2 s1
.source 2 s2

subusw d1, s1, s2


.function orc_subl
.dest 4 d1
.source 4 s1
.source 4 s2

subl d1, s1, s2


.function orc_subssl
.dest 4 d1
.source 4 s1
.source 4 s2

subssl d1, s1, s2


.function orc_subusl
.dest 4 d1
.source 4 s1
.source 4 s2

subusl d1, s1, s2


.function orc_subq
.dest 8 d1
.source 8 s1
.source 8 s2

subq d1, s1, s2


.function orc_sub_f32
.dest 4 d1 float
.source 4 s1 float

subf d1, d1, s1


.function orc_sub_f64
.dest 8 d1 double
.source 8 s1 double

subd d1, d1, s1


.function orc_andb
.dest 1 d1
.source 1 s1
.source 1 s2

andb d1, s1, s2


.function orc_andnb
.dest 1 d1
.source 1 s1
.source 1 s2

andnb d1, s1, s2


.function orc_andw
.dest 2 d1
.source 2 s1
.source 2 s2

andw d1, s1, s2


.function orc_andnw
.dest 2 d1
.source 2 s1
.source 2 s2

andnw d1, s1, s2


.function orc_andl
.dest 4 d1
.source 4 s1
.source 4 s2

andl d1, s1, s2


.function orc_andnl
.dest 4 d1
.source 4 s1
.source 4 s2

andnl d1, s1, s2


.function orc_andq
.dest 8 d1
.source 8 s1
.source 8 s2

andq d1, s1, s2


.function orc_andnq
.dest 8 d1
.source 8 s1
.source 8 s2

andnq d1, s1, s2
