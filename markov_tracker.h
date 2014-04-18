/*
 *  markov_tracker.h
 *
 *
 *  Created by Avinash on 2/8/10.
 *  Copyright 2010 Georgia Institute of Technology. All rights reserved.
 *
 */

//	1D Linked list of values to track certain values of the markov model
class tTrackerNode
	{
		
	private:
		
		unsigned int NumTokens; // Number of Tokens for a particular order model 
		unsigned int NumTypes; // Number of Types that have occurred once for a particular order model
		unsigned int NumTypes2; //Number of Types that have occurred twice for that order
		double EscapeProb; // Escape Probability for that Order model
		tTrackerNode * Prev; // Link to previous node
		tTrackerNode * Next; // Link to next node
		double Weight; //this is the weight for this particular level;
		double DiscountRatio;
		
		friend class tBundle2D;
		friend class cViewPoint;
		friend class tTracker;
		friend class _markov;
		
	public:
		
		//	Constructor
		tTrackerNode(unsigned int NumTokens, unsigned int NumTypes, double EscapeProb, tTrackerNode *Prev, tTrackerNode *Next);
		
		//	Destructor
		~tTrackerNode();
		
	};

class tTracker 
	{
		private:
	
			tTrackerNode * FirstNode; // Link to first node
			tTrackerNode * LastNode; // Link to last node
			unsigned int Order; // Keeps track of the order of the node
			unsigned int Length; // Number of nodes in the stream
			// Tracker parameters for x
			double Multiplier;
			double Offset;
			double Exponent;
		
			//parameters for gaussian smoothing
			double Mu;
			double Sigma;
	
			friend class tBundle2D;
			friend class tBundle2DHidden;
			friend class cViewPoint;
			friend class _markov;
	
		public:
	
			//	Constructor
			tTracker(tTrackerNode * FirstNode, tTrackerNode * LastNode, unsigned int Length);
			tTracker();
	
			//	Destructor
			~tTracker();
	
			//	GetNumTokens - return the number of tokens at a given order
			unsigned int GetNumTokens(unsigned int Order);
	
			//	GetNumTypes1 - return the number of types at a given order that have occurred exactly once
			unsigned int GetNumTypes(unsigned int Order);
	
			// GetNumTypes2 - return the number of types that have occurred exactly twice
			unsigned int GetNumTypes2(unsigned int Order);
	
			//	GetEscapeProb - return the EscapeProb at a given order
			double GetEscapeProb(unsigned int Order);
	
			//	GetLength - return the length of the stream
			unsigned int GetLength(void);
	
			//	Append - append a node at the end of the stream
			int	Append(void);
		
			unsigned int SetParameters(float a, float c, float x);
		
			unsigned int SetParameters(float a, float c);
	
			unsigned int GetOrder(void);
	
			unsigned int UpdateOrder(void);
		
			unsigned int SubtractOrder(void);
	
			unsigned int ResetOrder(void);
		
			unsigned int SetOrder(unsigned int order);
	
			unsigned int UpdateNumTypes(unsigned int Order);
	
			unsigned int UpdateNumTypes2(unsigned int Order);
	
			unsigned int UpdateNumTokens(unsigned int Order);
	
			unsigned int UpdateEscapeProb(unsigned int Order, void* x);
	
			unsigned int SubtractNumTypes(unsigned int Order);
		
			unsigned int SubtractNumTypes2(unsigned int Order);
		
			void SetWeight(unsigned int Order, double val);
		
			double GetWeight(unsigned int Order);
		
			unsigned int SetAllWeights(unsigned int length);
		
			unsigned int SetAllWeightsFunction(unsigned int length);
		
			unsigned int SetAllWeightsGaussian(unsigned int length);
			
			void DeleteAll(void);
		
			double GetDiscountRatio(unsigned int Order);
		
			unsigned int SetDiscountRatio(unsigned int Order, void * x);
	};