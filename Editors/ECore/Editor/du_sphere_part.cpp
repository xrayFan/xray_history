//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "du_sphere_part.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
Fvector du_sphere_part_vertices[DU_SPHERE_PART_NUMVERTEX]=
{
    -0.2887,	-0.2887,	0.2887,
    0.2887 ,	-0.2887,	0.2887,
    -0.2887,	0.2887 ,	0.2887,
    0.2887 ,	0.2887 ,	0.2887,
    0.0000 ,	0.0000 ,	0.5000,
    0.0000 ,	-0.3536,	0.3536,
    0.3536 ,	0.0000 ,	0.3536,
    0.0000 ,	0.3536 ,	0.3536,
    -0.3536,	0.0000 ,	0.3536,
    -0.2041,	-0.2041,	0.4082,
    0.2041 ,	-0.2041,	0.4082,
    0.2041 ,	0.2041 ,	0.4082,
    -0.2041,	0.2041 ,	0.4082,
    -0.1667,	-0.3333,	0.3333,
    0.0000 ,	-0.2236,	0.4472,
    -0.2236,	0.0000 ,	0.4472,
    -0.3333,	-0.1667,	0.3333,
    0.3333 ,	-0.1667,	0.3333,
    0.2236 ,	0.0000 ,	0.4472,
    0.1667 ,	-0.3333,	0.3333,
    0.1667 ,	0.3333 ,	0.3333,
    0.0000 ,	0.2236 ,	0.4472,
    0.3333 ,	0.1667 ,	0.3333,
    -0.3333,	0.1667 ,	0.3333,
    -0.1667,	0.3333 ,	0.3333,
    -0.2572,	-0.2572,	0.3430,
    -0.0981,	-0.2942,	0.3922,
    -0.1179,	-0.1179,	0.4714,
    -0.2942,	-0.0981,	0.3922,
    0.2572 ,	-0.2572,	0.3430,
    0.2942 ,	-0.0981,	0.3922,
    0.1179 ,	-0.1179,	0.4714,
    0.0981 ,	-0.2942,	0.3922,
    0.2572 ,	0.2572 ,	0.3430,
    0.0981 ,	0.2942 ,	0.3922,
    0.1179 ,	0.1179 ,	0.4714,
    0.2942 ,	0.0981 ,	0.3922,
    -0.2572,	0.2572 ,	0.3430,
    -0.2942,	0.0981 ,	0.3922,
    -0.1179,	0.1179 ,	0.4714,
    -0.0981,	0.2942 ,	0.3922,
    -0.2343,	-0.3123,	0.3123,
    -0.1857,	-0.2785,	0.3714,
    -0.2785,	-0.1857,	0.3714,
    -0.3123,	-0.2343,	0.3123,
    0.0000 ,	-0.3000,	0.4000,
    -0.1091,	-0.2182,	0.4364,
    -0.0870,	-0.3482,	0.3482,
    -0.1213,	0.0000 ,	0.4851,
    -0.2182,	-0.1091,	0.4364,
    0.0000 ,	-0.1213,	0.4851,
    -0.3482,	-0.0870,	0.3482,
    -0.3000,	0.0000 ,	0.4000,
    0.3123 ,	-0.2343,	0.3123,
    0.2785 ,	-0.1857,	0.3714,
    0.1857 ,	-0.2785,	0.3714,
    0.2343 ,	-0.3123,	0.3123,
    0.3000 ,	0.0000 ,	0.4000,
    0.2182 ,	-0.1091,	0.4364,
    0.3482 ,	-0.0870,	0.3482,
    0.1091 ,	-0.2182,	0.4364,
    0.1213 ,	0.0000 ,	0.4851,
    0.0870 ,	-0.3482,	0.3482,
    0.2343 ,	0.3123 ,	0.3123,
    0.1857 ,	0.2785 ,	0.3714,
    0.2785 ,	0.1857 ,	0.3714,
    0.3123 ,	0.2343 ,	0.3123,
    0.0000 ,	0.3000 ,	0.4000,
    0.1091 ,	0.2182 ,	0.4364,
    0.0870 ,	0.3482 ,	0.3482,
    0.2182 ,	0.1091 ,	0.4364,
    0.0000 ,	0.1213 ,	0.4851,
    0.3482 ,	0.0870 ,	0.3482,
    -0.3123,	0.2343 ,	0.3123,
    -0.2785,	0.1857 ,	0.3714,
    -0.1857,	0.2785 ,	0.3714,
    -0.2343,	0.3123 ,	0.3123,
    -0.2182,	0.1091 ,	0.4364,
    -0.3482,	0.0870 ,	0.3482,
    -0.1091,	0.2182 ,	0.4364,
    -0.0870,	0.3482 ,	0.3482,
    0.0000 ,	0.0000 ,	0.0000,
};
WORD du_sphere_part_faces[DU_SPHERE_PART_NUMFACES*3]=
{
     0, 41, 25,
    25, 44,  0,
    13, 42, 25,
    25, 41, 13,
     9, 43, 25,
    25, 42,  9,
    16, 44, 25,
    25, 43, 16,
     5, 45, 26,
    26, 47,  5,
    14, 46, 26,
    26, 45, 14,
     9, 42, 26,
    26, 46,  9,
    13, 47, 26,
    26, 42, 13,
     4, 48, 27,
    27, 50,  4,
    15, 49, 27,
    27, 48, 15,
     9, 46, 27,
    27, 49,  9,
    14, 50, 27,
    27, 46, 14,
     8, 51, 28,
    28, 52,  8,
    16, 43, 28,
    28, 51, 16,
     9, 49, 28,
    28, 43,  9,
    15, 52, 28,
    28, 49, 15,
     1, 53, 29,
    29, 56,  1,
    17, 54, 29,
    29, 53, 17,
    10, 55, 29,
    29, 54, 10,
    19, 56, 29,
    29, 55, 19,
     6, 57, 30,
    30, 59,  6,
    18, 58, 30,
    30, 57, 18,
    10, 54, 30,
    30, 58, 10,
    17, 59, 30,
    30, 54, 17,
     4, 50, 31,
    31, 61,  4,
    14, 60, 31,
    31, 50, 14,
    10, 58, 31,
    31, 60, 10,
    18, 61, 31,
    31, 58, 18,
     5, 62, 32,
    32, 45,  5,
    19, 55, 32,
    32, 62, 19,
    10, 60, 32,
    32, 55, 10,
    14, 45, 32,
    32, 60, 14,
     3, 63, 33,
    33, 66,  3,
    20, 64, 33,
    33, 63, 20,
    11, 65, 33,
    33, 64, 11,
    22, 66, 33,
    33, 65, 22,
     7, 67, 34,
    34, 69,  7,
    21, 68, 34,
    34, 67, 21,
    11, 64, 34,
    34, 68, 11,
    20, 69, 34,
    34, 64, 20,
     4, 61, 35,
    35, 71,  4,
    18, 70, 35,
    35, 61, 18,
    11, 68, 35,
    35, 70, 11,
    21, 71, 35,
    35, 68, 21,
     6, 72, 36,
    36, 57,  6,
    22, 65, 36,
    36, 72, 22,
    11, 70, 36,
    36, 65, 11,
    18, 57, 36,
    36, 70, 18,
     2, 73, 37,
    37, 76,  2,
    23, 74, 37,
    37, 73, 23,
    12, 75, 37,
    37, 74, 12,
    24, 76, 37,
    37, 75, 24,
     8, 52, 38,
    38, 78,  8,
    15, 77, 38,
    38, 52, 15,
    12, 74, 38,
    38, 77, 12,
    23, 78, 38,
    38, 74, 23,
     4, 71, 39,
    39, 48,  4,
    21, 79, 39,
    39, 71, 21,
    12, 77, 39,
    39, 79, 12,
    15, 48, 39,
    39, 77, 15,
     7, 80, 40,
    40, 67,  7,
    24, 75, 40,
    40, 80, 24,
    12, 79, 40,
    40, 75, 12,
    21, 67, 40,
    40, 79, 21,
    41,  0, 81,
     0, 44, 81,
    13, 41, 81,
    44, 16, 81,
     5, 47, 81,
    47, 13, 81,
    51,  8, 81,
    16, 51, 81,
    53,  1, 81,
     1, 56, 81,
    17, 53, 81,
    56, 19, 81,
     6, 59, 81,
    59, 17, 81,
    62,  5, 81,
    19, 62, 81,
    63,  3, 81,
     3, 66, 81,
    20, 63, 81,
    66, 22, 81,
     7, 69, 81,
    69, 20, 81,
    72,  6, 81,
    22, 72, 81,
    73,  2, 81,
     2, 76, 81,
    23, 73, 81,
    76, 24, 81,
     8, 78, 81,
    78, 23, 81,
    80,  7, 81,
    24, 80, 81,
};

WORD du_sphere_part_lines[DU_SPHERE_PART_NUMLINES*2]=
{
0,41,
0,44,
0,81,
1,53,
1,56,
1,81,
2,73,
2,76,
2,81,
3,63,
3,66,
3,81,
4,48,
4,50,
4,61,
4,71,
5,45,
5,47,
5,62,
5,81,
6,57,
6,59,
6,72,
6,81,
7,67,
7,69,
7,80,
7,81,
8,51,
8,52,
8,78,
8,81,
9,42,
9,43,
9,46,
9,49,
10,54,
10,55,
10,58,
10,60,
11,64,
11,65,
11,68,
11,70,
12,74,
12,75,
12,77,
12,79,
13,41,
13,42,
13,47,
13,81,
14,45,
14,46,
14,50,
14,60,
15,48,
15,49,
15,52,
15,77,
16,43,
16,44,
16,51,
16,81,
17,53,
17,54,
17,59,
17,81,
18,57,
18,58,
18,61,
18,70,
19,55,
19,56,
19,62,
19,81,
20,63,
20,64,
20,69,
20,81,
21,67,
21,68,
21,71,
21,79,
22,65,
22,66,
22,72,
22,81,
23,73,
23,74,
23,78,
23,81,
24,75,
24,76,
24,80,
24,81,
25,41,
25,42,
25,43,
25,44,
26,42,
26,45,
26,46,
26,47,
27,46,
27,48,
27,49,
27,50,
28,43,
28,49,
28,51,
28,52,
29,53,
29,54,
29,55,
29,56,
30,54,
30,57,
30,58,
30,59,
31,50,
31,58,
31,60,
31,61,
32,45,
32,55,
32,60,
32,62,
33,63,
33,64,
33,65,
33,66,
34,64,
34,67,
34,68,
34,69,
35,61,
35,68,
35,70,
35,71,
36,57,
36,65,
36,70,
36,72,
37,73,
37,74,
37,75,
37,76,
38,52,
38,74,
38,77,
38,78,
39,48,
39,71,
39,77,
39,79,
40,67,
40,75,
40,79,
40,80,
41,81,
44,81,
47,81,
51,81,
53,81,
56,81,
59,81,
62,81,
63,81,
66,81,
69,81,
72,81,
73,81,
76,81,
78,81,
80,81,
};       
         
         
         
         
         
         
         
         
         
         
         
         
