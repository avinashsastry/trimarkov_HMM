/*
 *  markov_Gaussian.cpp
 *  markov
 *
 *  Created by Avinash on 10/13/10.
 *  Copyright 2010 GTCMT. All rights reserved.
 *
 */

#undef check 

#include "markov_defines.h"
#include "markov_enums.h"
#include "markov_classes.h"
#include "markov_Gaussian.h"

#include "armadillo"

using namespace arma;


//  -------------------------------------------
//	Methods
//	=======
//
//	Updated: 14 Oct 2010, Avinash Sastry, GTCMT
//	-------------------------------------------

tGaussian :: tGaussian()
{
	// For a new gaussian distribution,
	// initialize everything to 0
	
	for(unsigned int i = 0; i<NUM_FEATURES; i++)
	{
		Mu[i] = 0.0;
		
		for(unsigned int j = 0; j<NUM_FEATURES; j++)
		{
			Sigma[i][j] = 0.0;
		}
	}
}

tGaussian :: tGaussian(float* MeanVector, float* Variance, float* Inverse, float* Determinant)
{
	unsigned int i = 0;
	unsigned int j = 0;
	
	// Initialize Mu
	for(i = 0; i<NUM_FEATURES; i++)
	{
		Mu[i] = *(MeanVector+i);
	}
	
	// Initialize Sigma
	for(j = 0; j<NUM_FEATURES; j++)
	{
		for(i = 0; i<NUM_FEATURES; i++)
		{
			Sigma[j][i] = *(Variance + j*NUM_FEATURES + i);
		}
	}
	
	// Initialize SigmaInverse
	for(j = 0; j<NUM_FEATURES; j++)
	{
		for(i = 0; i<NUM_FEATURES; i++)
		{
			SigmaInverse[j][i] = *(Inverse + j*NUM_FEATURES + i);
		}
	}
	
	// Initialize DetSigma
	DetSigma = *Determinant;
}

tGaussian :: ~tGaussian()
{
}


double tGaussian :: GetLikelihood(float* FeatureVector, unsigned int Length)
{	
	double Prob = 0;
	
	double A = 0;
	double B = 0;
	double Determinant = 0;
	double Term = 0;
	
	mat MuMat(NUM_FEATURES, 1);
	mat Features(NUM_FEATURES, 1);
	mat SigmaMat(NUM_FEATURES, NUM_FEATURES);
	mat InverseMat(NUM_FEATURES, NUM_FEATURES);
	mat TransMat(1, NUM_FEATURES);
	mat D(1, NUM_FEATURES);
	mat E(NUM_FEATURES, 1);
	mat C(1, 1);
	
	// Check for FeatureVector's size
	if ((Length == Features.n_rows))
	{
		// initialize MuMat and SigmaMat and Sigma Inverse
		for (unsigned int i = 0; i < Length; i++)
		{
			MuMat(i, 0) = Mu[i];
			
			for(unsigned int j = 0; j < Length; j++)
			{
				SigmaMat(i, j) = Sigma[i][j];
				InverseMat(i, j) = SigmaInverse[i][j];
			}
		}
		// initialize Sigma
		// initialize Features to the values of the FeatureVector
		for (unsigned int i = 0; i < Length; i++) {
			Features(i, 0) = *(FeatureVector+i);
		}
		
		TransMat = trans(Features-MuMat);
		
		D = TransMat*InverseMat;
		E = Features-MuMat;
		
		C = D*E;
		
		Term = C(0,0);
		Determinant = det(SigmaMat);
				
		A = exp(Term * (-0.5));
		B = pow(2*M_PI, (double)NUM_FEATURES/(double)2)*sqrt(Determinant);
		
		Prob = A/B;
	}
	return Prob;
}




