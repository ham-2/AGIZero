#ifndef CONST_INCLUDED
#define CONST_INCLUDED

namespace AGI {
	constexpr int SEARCH_THREADS_DEFAULT = 1;
	constexpr int SEARCH_THREADS_MAX = 128;
	constexpr int NULL_DEPTH = 1;
	constexpr int QUIESCENCE_DEPTH = 2;
	constexpr int DEFAULT_PLY = 2;
	constexpr int MIDDLEGAME_DELTA = 100;
	constexpr int ENDGAME_DELTA = 500;
	constexpr int INCREMENT_DELTA = 22;

	constexpr int TABLE_MB_DEFAULT = 64;
	constexpr int TABLE_MB_MAX = 4096;
	constexpr int PAWN_TABLE_MB_DEFAULT = 4;
	constexpr int PAWN_TABLE_MB_MAX = 64;

	constexpr float DEFAULT_TIME = 3.0f;
	constexpr float DEFAULT_TIME_MAX = 5.0f;
	constexpr int PRINT_MIN_MS = 2000;
	constexpr int PRINT_ATM_MS = 100;
	constexpr int MAX_PLY = 100;

	constexpr int ATM_MIN_TIME = 5;
	constexpr int ATM_STABILIZATION = 15;
	constexpr int ATM_FLEXIBILITY = 70;
	constexpr int ATM_MG_BIAS = 100;

	constexpr int PERFT_ITER = 15;
	constexpr int PERFT_THREAD_I = 4;
	constexpr int PERFT_THREAD[PERFT_THREAD_I] = { 1, 2, 4, 8 };
	constexpr int PERFT_DEPTH_J = 1;
	constexpr int PERFT_DEPTH[PERFT_DEPTH_J] = { 12 };

	constexpr int SEARCH_DEPTH_BIAS = 20;
	constexpr int DELTA_MARGIN = 100;
	constexpr int CLOSE_EXT_MARGIN = 30;
	constexpr int CHANGING_EXT_MARGIN = 100;
	constexpr int FUTILITY_2_MARGIN = 1200;
	constexpr int FUTILITY_1_MARGIN = 800;

	// Model
	constexpr int PS[14] = { 39, 28, 24, 7, 14, 8, 12, 29, 12, 19, 18, 20, 16, 11 };
	constexpr int MG[8] = { 16, 21, 8, 10, 10, 13, 0, 12 }; // 4, 5, 7th not used
	constexpr int EG[4] = { 12, 9, 5, 19 };
	constexpr int LD[4] = { 3, 1, 19, 7 }; // 2nd not used
	constexpr int PN[8] = { 8, 25, 5, 36, 11, 16, 12, 26 };
	constexpr int KS[12] = { 3, 3, 4, 1, 5, 2, 11, 3, 1, 2, 1, 8 };

	constexpr int KNOWN_WIN =  15000;
	constexpr int EVAL_WIN  =  30000;
	constexpr int EVAL_LOSS = -30000;

	constexpr int King_Attack[64] = {
			   0,    2,    3,    8,   13,
			  17,   23,   30,   38,   45,
			  55,   66,   78,   92,  106,
			 122,  139,  156,  175,  195,
			 216,  239,  267,  297,  328,
			 361,  395,  431,  469,  508,
			 548,  591,  634,  680,  727,
			 775,  825,  877,  930,  984,
			1041, 1098, 1158, 1219, 1281,
			1345, 1411, 1478, 1547, 1617,
			1689, 1763, 1838, 1914, 1992,
			2063, 2094, 2117, 2136, 2153,
			2172, 2181, 2189, 2192
	};

	constexpr int Reductions[101] = {
		0,	0,	1,	1,	1,
		1,	2,	2,	2,	3,
		3,	3,	4,	4,	4,
		5,	5,	6,	7,	8,
		8,	10,	10,	11,	12,
		13,	13,	15,	15,	16,
		17,	18,	18,	20,	20,
		21,	22,	23,	23,	25,
		25,	26,	27,	28,	28,
		30,	30,	31,	32,	33,
		33,	35,	35,	36,	37,
		38,	38,	40,	40,	41,
		42,	43,	43,	45,	45,
		46,	47,	48,	48,	50,
		50,	51,	52,	53,	53,
		55,	55,	56,	57,	58,
		58,	60,	60,	61,	62,
		63,	63,	65,	65,	66,
		67,	68,	68,	70,	70,
		71,	72,	73,	73,	75,
		75
	};

	constexpr int DEPTH_BIASES[101] = {
		  0,	  3,	  5,	  7,	  9,
		 11,	 12,	 14,	 15,	 16,
		 17,	 18,	 20,	 22,	 24,
		 25,	 27,	 29,	 32,	 34,
		 36,	 38,	 40,	 43,	 45,
		 47,	 49,	 52,	 54,	 56,
		 59,	 61,	 64,	 66,	 68,
		 71,	 73,	 76,	 78,	 81,
		 83,	 86,	 88,	 91,	 93,
		 96,	 98,	101,	104,	106,
		109,	111,	114,	117,	119,
		122,	125,	127,	130,	133,
		136,	138,	141,	144,	147,
		149,	152,	155,	158,	160,
		163,	166,	169,	172,	175,
		177,	180,	183,	186,	189,
		192,	195,	197,	200,	203,
		206,	209,	212,	215,	218,
		221,	224,	227,	230,	233,
		236,	239,	242,	245,	248,
		251
	};


	constexpr int Primes[] = {
2731,	2741,	2749,	2753,	2767,	2777,	2789,	2791,	2797,	2801,
2803,	2819,	2833,	2837,	2843,	2851,	2857,	2861,	2879,	2887,
2897,	2903,	2909,	2917,	2927,	2939,	2953,	2957,	2963,	2969,
2971,	2999,	3001,	3011,	3019,	3023,	3037,	3041,	3049,	3061,
3067,	3079,	3083,	3089,	3109,	3119,	3121,	3137,	3163,	3167,
3169,	3181,	3187,	3191,	3203,	3209,	3217,	3221,	3229,	3251,
3253,	3257,	3259,	3271,	3299,	3301,	3307,	3313,	3319,	3323,
3329,	3331,	3343,	3347,	3359,	3361,	3371,	3373,	3389,	3391,
3407,	3413,	3433,	3449,	3457,	3461,	3463,	3467,	3469,	3491,
3499,	3511,	3517,	3527,	3529,	3533,	3539,	3541,	3547,	3557,
3559,	3571,	3581,	3583,	3593,	3607,	3613,	3617,	3623,	3631,
3637,	3643,	3659,	3671,	3673,	3677,	3691,	3697,	3701,	3709,
3719,	3727,	3733,	3739,	3761,	3767,	3769,	3779,	3793,	3797,
3803,	3821,	3823,	3833,	3847,	3851,	3853,	3863,	3877,	3881,
3889,	3907,	3911,	3917,	3919,	3923,	3929,	3931,	3943,	3947,
3967,	3989,	4001,	4003,	4007,	4013,	4019,	4021,	4027,	4049,
4051,	4057,	4073,	4079,	4091,	4093,	4099,	4111,	4127,	4129,
4133,	4139,	4153,	4157,	4159,	4177,	4201,	4211,	4217,	4219,
4229,	4231,	4241,	4243,	4253,	4259,	4261,	4271,	4273,	4283,
4289,	4297,	4327,	4337,	4339,	4349,	4357,	4363,	4373,	4391
	};
	
}

#endif
