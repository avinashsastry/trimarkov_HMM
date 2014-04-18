/*
 *  markov_Gaussian.h
 *  markov
 *
 *  Created by Avinash on 10/13/10.
 *  Copyright 2010 GTCMT. All rights reserved.
 *
 */

#define NUM_FEATURES	20

//  -------------------------------------------
//	Documentation
//	-------------------------------------------
//	
//	Syntax
//	======
//
//	Purpose
//	=======
//
//	TGAUSSIAN describes a d-dimensional gaussian 
//  distribution with parameters Mu and Sigma. 
//	Mu represents the vector of mean values along
//	each dimension and Sigma is the matrix of 
//	co-variances.
//
//	NOTE: Uses Armadillo for matrix operations.
//
//	Fields
//	======
//	
//	Mu				-	Armadillo matrix (1x3)
//	Sigma			-	Armadillo matrix (3x3)
//
//  -------------------------------------------

//  -------------------------------------------
//	Declaration
//
//	Updated: 13 Oct 2010, Avinash Sastry, GTCMT
//	-------------------------------------------


class tGaussian {
private:
	float Mu[NUM_FEATURES];
	float Sigma[NUM_FEATURES][NUM_FEATURES];
	float DetSigma;
	float SigmaInverse[NUM_FEATURES][NUM_FEATURES];
	
public:
	
	tGaussian();
	
	tGaussian(float* MeanVector, float* Variance, float* Inverse, float* Determinant);
	
	~tGaussian();
	
	//  -------------------------------------------
	//	Function - GetLikelihood()
	//	-------------------------------------------
	//	
	//	Syntax
	//	======
	//	
	//	double Answer = GetLikelihood(float*, unsigned int)
	//
	//	Purpose
	//	=======
	//
	//	Returns the likelihood, and not the probability, of a given point in
	//	d-dimensional space within the gaussian
	//	distribution.
	//
	//	NOTE: Uses Armadillo library for matrix operations.
	//
	//	Arguments
	//	=========
	//	
	//	FeatureVector	-	pointer to double array[3]
	//						representing a point
	//						in space.
	//						eg: (2.3, 4.5, 1.2)
	//
	//	Length			-	unsigned int
	//						length of feature vector
	//
	//  -------------------------------------------
	
	double GetLikelihood(float* FeatureVector, unsigned int Length);
};

