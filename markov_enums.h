/*
 *  markov_enums.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

//	Enumeration for western pitch classes
enum ePitchClass
{
	ePITCH_CLASS_C,
	ePITCH_CLASS_Cs,
	ePITCH_CLASS_Db = 1,
	ePITCH_CLASS_D,
	ePITCH_CLASS_Ds,
	ePITCH_CLASS_Eb = 3,
	ePITCH_CLASS_E,
	ePITCH_CLASS_F,
	ePITCH_CLASS_Fs,
	ePITCH_CLASS_Gb = 6,
	ePITCH_CLASS_G,
	ePITCH_CLASS_Gs,
	ePITCH_CLASS_Ab = 8,
	ePITCH_CLASS_A,
	ePITCH_CLASS_As,
	ePITCH_CLASS_Bb = 10,
	ePITCH_CLASS_B
};

//	Enumeration for return/status/debug values/tokens
enum eRETVAL
{
	eRETVAL_SUCCESS				= 0x0000,
	eRETVAL_FAILURE				= 0x0001,
	eRETVAL_1DNODE_CREATED		= 0x0002,
	eRETVAL_1DNODE_UPDATED		= 0x0004,
	eRETVAL_2DNODE_CREATED		= 0x0008,
	eRETVAL_2DNODE_UPDATED		= 0x0010,
	eRETVAL_BRANCH_CREATED		= 0x0020,
	eRETVAL_BRANCH_UPDATED		= 0x0040,
	eRETVAL_PRIOR_ALONE_USED	= 0x0080,
	eRETVAL_PREDICTION_MADE		= 0x0100,
	eRETVAL_NULL_INPUT			= 0x0200,
	eRETVAL_ESCAPE_USED			= 0x0400,
	eRETVAL_UNDEFINED			= 0x8000
};

// Enumberation for pitch tracking states
enum ePT_STATE
{
	ePT_STATE_GOING_UP,
	ePT_STATE_GOING_DOWN,
	ePT_STATE_GOING_FLAT,
	ePT_STATE_RESET
};