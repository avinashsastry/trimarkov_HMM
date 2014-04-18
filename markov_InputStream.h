/*
 *  markov_InputStream.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */
#ifndef _MARKOV_INPUTSTREAM
#define _MARKOV_INPUTSTREAM
//	Linked list of input note stream
class tInputStream : public c1DList
	{
		
	private:
		
		friend class tBundle2D;
		friend class _markov;
		
	public:
		
		/* //	Constructor
		tInputStream(t1DNode *FirstNode, t1DNode *LastNode, unsigned int Length);
		
		//	Destructor
		~tInputStream();*/
	};
#endif