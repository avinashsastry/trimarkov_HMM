/*
 *  markov_defines.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/8/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

//	Define MAX4 or MAX5 as a preprocessor macro depending on the intended target platform;
//	MAX5 is the default.
#if !(defined MAX4 | defined MAX5)
	#define	MAX5
#endif

// Post routines to Max Window - for debugging purpose
#if	defined		MAX4
	#define POST(arg1,...)					post(__VA_ARGS__)
#elif defined	MAX5
	#define	POST							object_post
#endif

//	Preprocessor definitions
#define	NUM_PITCH_CLASSES	12

#define	MAX_NUM_PITCH_CLASS_NAMES	2
#define	MAX_PITCH_CLASS_NAME_LEN	2

#define	NULL_INDEX			255
#define	NULL_REV_INDEX		255
#define	PROXY_ALLOWANCE		2

#define SECONDS_PER_MINUTE	60

#define MAX_ORDER			3
#define IS_SHORT_TERM_MODEL	1
#define	BARS_PER_SONG		16

#define MAX_STROKES			42
#define USE_SHORT_TERM		0

#define NUM_FEATURES		20
#define NUM_STROKES			9

#define THETA				0.01
