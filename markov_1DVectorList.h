/*
 *  markov_1DVectorList.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/14/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

//	Linked list of 1D vector nodes
class c1DVectorList
	{
		
	protected:
		
		c1DVectorNode	* FirstNode; // Link to first node
		c1DVectorNode	* LastNode; // Link to last node
		unsigned int	Length; // Number of nodes in the stream
		unsigned int	Dim; // Number of dimensions in the vectors within
		
		friend class tBundle2D;
		friend class tBundle2DHidden;
		friend class cQaidaControl;
		friend class _markov;
		
	public:
		
		//	Constructor
		c1DVectorList();
		
		//	Constructor
		c1DVectorList(unsigned int Dim, unsigned int Length, c1DVectorNode *FirstNode, c1DVectorNode *LastNode);
		
		//	Destructor
		~c1DVectorList();
		
		//	GetVector - return the list at given index
		c1DList *GetVector(unsigned int Index);
		
		//	GetValue - return the value at given index
		float GetValue(unsigned int VectorIndex, unsigned int DimIndex);
		
		//	GetLength - return the length of the list
		unsigned int GetLength(void);
		
		//	SetLength - set the length of the list
		void SetLength(unsigned int Length);
		
		//	GetDim - return the number of dimensions fo the vectors of the list
		unsigned int GetDim(void);
		
		//	SetDim - set the number of dimensions fo the vectors of the list
		void SetDim(unsigned int Dim);
		
		//	Append - append a node at the end of the list
		int	Append(c1DList *Vector);
		
		//	Delete - delete a node
		int Delete(unsigned int Index);
		
		//	DeleteAll - delete all nodes
		int DeleteAll(void);
		
		//	AppendAll - append all nodes from another c1DVectorList; in effect, combine them
		int	AppendAll(c1DVectorList *OtherList);
		
		//	Assign - copy list from another c1DVectorList
		int	Assign(c1DVectorList *OtherList);
		
		//	LeftShift - Push all nodes to the left, so that the first node goes out
		//  Used to maintain the max order limit
		int LeftShift(void);
		
		//  Add a node with a vector of Features - used for continuous model
		//  TODO: Improve this function, make it a bit more organized.
		unsigned int AddFeatures(float* FeatureVector, c1DList* Vector);
		
		//	Overloaded '=' for another pointer
		c1DVectorList *operator = (c1DVectorList *OtherList);
		
		//	Overloaded '=' for another object
		c1DVectorList *operator = (c1DVectorList OtherList);
		
		//	Output the list - one vector at a time
		//	For debugging only; may impair performance
		void Post(void *pOutlet);
	};
