/*
 *  markov_2DNodeHidden.h
 *  markov
 *
 *  Created by Avinash on 9/30/10.
 *  Copyright 2010 GTCMT. All rights reserved.
 *
 */


//  t2DNodeHidden represents a node in a hidden markov system.
//  There are two options for this node:
//  1. It can be a discrete emission node - meaning the visible symbols are discrete values
//  2. Or it can be a continuous distribution node - the symbols can be in a continuous range

//  For my current purpose, I am turning it into a continuous emission node
//  To use it as a discrete emission node, uncomment the tBundle2D Emission field
//  That serves as an array of discrete values.

class t2DNodeHidden
{
	
private:
	
	t2DNodeHidden				*Left; // Link to the 2D node to the left
	t2DNodeHidden				*Right; // Link to the 2D node to the right
	tBundle2DHidden				*Bundle; // Link to the bundle (of 2D nodes) that contains this 2D node
	tBundle2DHidden				*Bottom; // Link to the child bundle
	c1DList						Vector; // Vector of symbol values
	unsigned long int			Count; // Number of times the node has been observed in the input
	double						Prob; // Corresponding probability of occurrence - this is the a_ij(t)
	
	//  For discrete emission case
	tBundle2D					Emission; // Emission probabilities for possible visible symbols -> b_jk(t)
	
	// For continuous emission case
	tGaussian					EmissionDistribution;
		
	friend class tBundle2DHidden;
	friend class cViewPoint;
	friend class cViewPointHidden;
	friend class cQaidaControl;
	friend class _markov;
	
public:
	
	//	Constructor
	t2DNodeHidden(t2DNodeHidden *Left, t2DNodeHidden *Right, tBundle2DHidden *Bundle, tBundle2DHidden *Bottom, c1DList *Vector, unsigned long int Count, double Prob);
	
	t2DNodeHidden(t2DNodeHidden *Left, t2DNodeHidden *Right, tBundle2DHidden *Bundle, tBundle2DHidden *Bottom, c1DList *Vector, unsigned long int Count, double Prob, float* MeanVector, float* Variance, float* Inverse, float* Determinant);
	
	//	Destructor
	~t2DNodeHidden();
	
	unsigned int Emit(c1DList *outcome, void *x);
	
	unsigned int UpdateSymbol(c1DList *SymbolVector, void *x);
	
	double EmissionProbability(c1DList *Vector);
	
	// Returns the likelihood of a feature vector through the gaussian in the node
	double GaussianLikelihood(float* Features);
};

	
