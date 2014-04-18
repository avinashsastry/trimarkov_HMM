/*
 *  markov_1DVectorNode.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/14/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

//	Node of a 1D linked list containing 1D vectors in each node
class c1DVectorNode
	{
		
	private:
		
		c1DList			Vector; // Node vector
		c1DVectorNode	*Prev; // Link to previous node
		c1DVectorNode	*Next; // Link to next node
		float			FeatureVector[NUM_FEATURES];
		
		friend class	c1DVectorList;
		friend class	tBundle2D;
		friend class	tBundle2DHidden;
		friend class	_markov;
		
	public:
		
		//	Constructor
		c1DVectorNode(c1DList *Vector, c1DVectorNode *Prev, c1DVectorNode *Next);
		
		//	Destructor
		~c1DVectorNode();
		
	};
