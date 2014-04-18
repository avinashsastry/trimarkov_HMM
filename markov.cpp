/*
	markov -	A Max object that implements a Markov chain / trie / prefix tree for musical features
				upto a specified or infinite order, and providing for other features like backtracking, etc.
 
				Dynamic & arguably efficient - uses linked lists for memory structures & the classes
				are custom written. It might however be better to use a standard library like Boost,
				VL (Vector Library) &/or data structure libraries for C++. Uses recursion to trace
				the structure wherever appropriate.
	
	trishul.mallikarjuna@gatech.edu
	Center for Music Technology
	Georgia Institute of Technology
	Atlanta, GA, USA
	http://gtcmt.gatech.edu
*/
#undef check
//	Max includes
#include "ext.h"
#include "ext_obex.h"

//	C/CPP includes
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>

//	Project includes
#include "markov_defines.h"
#include "markov_enums.h"
#include "markov_classes.h"
#include "markov_1DNode.h"
#include "markov_1DList.h"
#include "markov_1DVectorNode.h"
#include "markov_1DVectorList.h"
#include "markov_Bundle2D.h"
#include "markov_2DNode.h"
#include "markov_InputStream.h"
#include "markov_InputVectorStream.h"
#include "markov_tracker.h"

#include "armadillo"
#include "markov_Gaussian.h"

#include "markov_2DNodeHidden.h"
#include "markov_Bundle2DHidden.h"
#include "markov_ViewPoint.h"
#include "markov_ViewpointHidden.h"
#include "markov_qaidaControl.h"

using namespace std;
using namespace arma;

//	Global class pointer variable
void *markov_class;

//	Object class
typedef class _markov 
{
public:
	t_object					ob;
	t_atom						val;
	t_symbol					*name;
	long int					in;

	//	Inlets
	void						*pIn2;
	void						*pInBeat;
	void						*pInMeasure;
	
	//	Outlets
	void						*pOut;
	void						*pOutStream;
	void						*pOutCount;
	void						*pOutProb;
	
	//	Data structure variables
	cViewPointHidden			vpNotes;
	
} t_markov;

//	Function prototypes
	//	Standard set
void	*markov_new(t_symbol *s, long argc, t_atom *argv);
void	markov_free(t_markov *x);
void	markov_assist(t_markov *x, void *b, long m, long a, char *s);

void	markov_int(t_markov *x, long n);
void	markov_list(t_markov *x, t_symbol *s, long ac, t_atom *av);
void	markov_init(t_markov *x, t_symbol *s, long ac, t_atom *av);
void	markov_float(t_markov *x, double f);
void	markov_anything(t_markov *x, t_symbol *s, long ac, t_atom *av);
int		markov_bang(t_markov *x);
void	markov_identify(t_markov *x);
void	markov_dblclick(t_markov *x);
void	markov_acant(t_markov *x);

	//	Custom set
void	markov_set(t_markov *x, t_symbol *s, long ac, t_atom *av);
void	markov_in2(t_markov *x, t_symbol *s, long ac, t_atom *av);

unsigned int markov_beat( void* x);
void PopulateGaussians(cViewPointHidden* VP, unsigned int SetNumber);
void CreateDummyTree(t_markov* x);


//	Main routine
int main(int argc, char * argv[])
{	
	int		retval = 1;
	
#ifdef	MAX4

	// object initialization, OLD STYLE
	setup((t_messlist **)&markov_class, (method)markov_new, (method)markov_free, (short)sizeof(t_markov), 
				0L, A_GIMME, 0);
	
    addmess((method)markov_bang,			(char *)"bang",			0);
    addmess((method)markov_int,				(char *)"int",			A_LONG, 0);  
    addmess((method)markov_list,			(char *)"list",			A_GIMME, 0);  
    addmess((method)markov_init,			(char *)"init",			A_GIMME, 0);
	addmess((method)markov_float,			(char *)"float",		A_FLOAT, 0);  
    addmess((method)markov_anything,		(char *)"anything",		A_GIMME, 0);  
    addmess((method)markov_identify,		(char *)"identify",		0);
	addmess((method)markov_acant,			(char *)"blooop",		A_CANT, 0);	
    addmess((method)markov_assist,			(char *)"assist",		A_CANT, 0);  
    addmess((method)markov_dblclick,		(char *)"dblclick",		A_CANT, 0);
    addmess((method)markov_set,				(char *)"set",			A_GIMME, 0);
	addmess((method)markov_in2,				(char *)"in2",			A_GIMME, 0);

	class_register(CLASS_BOX, (t_class *)markov_class);

#elif	defined MAX5

	// object initialization, NEW STYLE
	t_class *c;
	c = class_new("markov", (method)markov_new, (method)markov_free, (long)sizeof(t_markov), 
					0L /* leave NULL!! */, A_GIMME, 0);
	
    class_addmethod(c, (method)markov_bang,				"bang",			0);
    class_addmethod(c, (method)markov_int,				"int",			A_LONG, 0);  
    class_addmethod(c, (method)markov_list,				"list",			A_GIMME, 0);  
    class_addmethod(c, (method)markov_init,				"init",			A_GIMME, 0);
	class_addmethod(c, (method)markov_float,			"float",		A_FLOAT, 0);  
    class_addmethod(c, (method)markov_anything,			"anything",		A_GIMME, 0);  
    class_addmethod(c, (method)markov_identify,			"identify",		0);
	CLASS_METHOD_ATTR_PARSE(c, "identify", "undocumented", gensym("long"), 0, "1");
	class_addmethod(c, (method)markov_acant,			"blooop",		A_CANT, 0);	
	CLASS_METHOD_ATTR_PARSE(c, "blooop", "documentable", gensym("long"), 0, "1");

	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)markov_assist,			"assist",		A_CANT, 0);  
    class_addmethod(c, (method)markov_dblclick,			"dblclick",		A_CANT, 0);
    class_addmethod(c, (method)markov_set,				"set",			A_GIMME, 0);
	class_addmethod(c, (method)markov_in2,				"in2",			A_GIMME, 0);

	CLASS_ATTR_SYM(c, "name", 0, t_markov, name);
	
	class_register(CLASS_BOX, c);
	markov_class = c;

#endif

	retval = 0;
	return retval;
}

//
void markov_acant(t_markov *x)
{
	POST((t_object *)x, (char *)"can't touch this!");
}

//	Inlet & outlet mouseover descriptions
void markov_assist(t_markov *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { //inlet
		switch (a) {
			case 0:
				sprintf(s, "Training Input");
				break;
			case 1:
				sprintf(s, "Testing input for discrete");
				break;
			case 2:
				sprintf(s, "Testing input for continuous");
				break;
			case 3:
				sprintf(s, "<blank>");
				break;
			default:
				sprintf(s, "Another input");
				break;
		}
	} 
	else {	// outlet
		switch (a) {
			case 0:
				sprintf(s, "Main output: synthesized symbol");
				break;
			case 1:
				sprintf(s, "List: input stream");
				break;
			case 2:
				sprintf(s, "List: counts");
				break;
			case 3:
				sprintf(s, "List: probabilities");
				break;
			default:
				sprintf(s, "Another output");
				break;
		}			
	}
}

//	Clean-up at object destruction
void markov_free(t_markov *x)
{
	;
}

//	For double-click
void markov_dblclick(t_markov *x)
{
	POST((t_object *)x, (char *)"I got a double-click!");
}

//	For integer input - most common while modeling a musical note sequence
void markov_int(t_markov *x, long n)
{
	unsigned int SetNumber = n;
	atom_setlong(&x->val, n);
	
	t_atom atom;
	atom_setlong(&atom, n);
	
	//POST((t_object*)x, (char*)"Training Set %d", SetNumber);
	PopulateGaussians(&x->vpNotes, SetNumber);
}

//	For float input
void markov_float(t_markov *x, double f)
{
	//POST((t_object *)x, (char *)"float: %f",f);
	
	atom_setfloat(&x->val, f);
	
	t_atom atom;
	atom_setfloat(&atom, f);
	
	//	Pass the float as a single-entry list to markov_list to do further processing
	markov_list(x, gensym((char *)"list"), 1, &atom);
}

//	For list input
void markov_list(t_markov *x, t_symbol *s, long ac, t_atom *av)
{	
	float InputState = atom_getfloat(av);
	float FeatureVector[NUM_FEATURES];
	double Entropy = 0;
	double ForwardProb = 0;
	
	c1DList * DataToken = new c1DList();
	c1DList * Outcome = new c1DList();
	
	switch (proxy_getinlet((t_object *)x))
	{
		case 0:
			atom_setsym(&x->val, s);
			
			// This is the primary inlet. We'll use this for our training input.
			
			// Input: Only the symbol is given for this training.
			// The features are fed into the Gaussian externally.
			
			// The input state is added to the InputStateStream.
			// Then, the contents of the StateStream are fed to the tree.
			
			x->vpNotes.AddToStream(InputState, InputState, x);
			break;
			
		case 1:
			// This is the testing input.
			// Here only one input is needed, the visible symbol.
			// This is fairly simple, the symbol is just added to the 
			// Search Stream so that it can be looked up.
			//x->vpNotes.AddToSearchStream(InputSymbol, x);
			
			// NOTE: Yes, I know it says InputState, it just takes in the 
			// first element of the list. This should be the SYMBOL and not the state.
			
			// We will also add some cross-validation later on.
			
			
			break;
		case 2:
			// Continuous input
			// Set the stroke to be cross-validated:
			DataToken->Append(atom_getfloat(av));
			
			// A feature vector of some length (23, for example) is entered as a list
			for(unsigned int i = 0; i < NUM_FEATURES; i++)
			{
				FeatureVector[i] = atom_getfloat(av+i+1);
			}
			
			POST((t_object*)x, " - - - Input Stroke: %f - - - ", DataToken->GetValue(0));
			
			//Add current stroke to the search stream				
			x->vpNotes.AddFeaturesToSearchStream(FeatureVector, DataToken);
			
			// Testing Procedure
			// 1. Evaluate forward probability for each sequence
			// 2. Sum them together to get a full distribution
			// 3. Crossvalidate
			
			// Evaluation includes cross-validation in this function
			// Calculates the probability of the given sequence having occurred
			x->vpNotes.EvaluateStreamContinuous(x);
			x->vpNotes.PrevPDF->Normalize();
			
			POST((t_object*)x, "Evaluating Stream");
			x->vpNotes.PrevPDF->PrintBundle(x);
		
			// 1. Adjust the weights in the trie
			x->vpNotes.ForwardBackwardAlgorithm(x);
			
			// 2. Evaluate the stream
			x->vpNotes.EvaluateStreamContinuous(x);
			x->vpNotes.PrevPDF->Normalize();
			
			// 3. Classify the current stroke
			x->vpNotes.PrevPDF->ClassifyFeatures(FeatureVector, Outcome);
			
			// 4. Get Entropy and crossvalidate
			Entropy = x->vpNotes.PrevPDF->GetProbability(FeatureVector, DataToken, &x->vpNotes.Tracker);
			
			POST((t_object*)x, "Evaluating Stream after FwdBwd()");
			x->vpNotes.PrevPDF->PrintBundle(x);
			
			if (Entropy != 0)
			{
				Entropy = -log(Entropy)/log(2);
			}
			else 
			{
				POST((t_object*)x, "Zero Probability returned - Tree does not exist");
				Entropy = x->vpNotes.Tracker.GetEscapeProb(0);
			}

			
			// Output this entropy value from the second inlet
			av = new t_atom[3];
			atom_setfloat(av, DataToken->GetValue(0));
			atom_setfloat(av+1, Entropy);
			atom_setfloat(av+2, Outcome->GetValue(0));
			outlet_list(x->pOut, 0L, 3, av);
			break;
		case 3:
			break;
		default:
			break;
	}

}

//	For input in inlet 2
void markov_in2(t_markov *x, t_symbol *s, long ac, t_atom *av)
{
	atom_setsym(&x->val, s);
	//	x->RootPitchClass = (unsigned short int) ac;
	POST((t_object *)x, (char *)"Input received in inlet 2!");
	POST((t_object *)x, (char *)"ac=%ld; av=%ld",ac,av);
	markov_bang(x);
}

//	Setter
void markov_set(t_markov *x, t_symbol *s, long ac, t_atom *av)
{
	POST((t_object *)x, (char *)"set; ac=%ld",ac);
	t_symbol *VarName;
	
	if(ac>0)
	{
		atom_arg_getsym(&VarName, 0, ac, av);
		//	TODO: tolower
		if(!strcmp(VarName->s_name, "tonic"))
		{
			long int Tonic;
			atom_arg_getlong(&Tonic, 1, ac, av);
			//x->Features.SetTonic(Tonic);
		}
		else if(!strcmp(VarName->s_name, "tempo"))
		{
			float Tempo;
			atom_arg_getfloat(&Tempo, 1, ac, av);
			//x->Features.SetTempo(Tempo);
		}
		else if(!strcmp(VarName->s_name, "beatsperbar"))
		{
			long int BeatsPerBar;
			atom_arg_getlong(&BeatsPerBar, 1, ac, av);
			//x->Features.SetBeatsPerBar(BeatsPerBar);
		}
		else if(!strcmp(VarName->s_name, "tataperbeat"))
		{
			long int TataPerBeat;
			atom_arg_getlong(&TataPerBeat, 1, ac, av);
			//x->Features.SetTataPerBeat(TataPerBeat);
		}
		else if(!strcmp(VarName->s_name, "numoctaves"))
		{
			long int NumOctaves;
			atom_arg_getlong(&NumOctaves, 1, ac, av);
			//x->Features.SetNumOctaves(NumOctaves);
		}
		else if(!strcmp(VarName->s_name, "octaveoffset"))
		{
			long int OctaveOffset;
			atom_arg_getlong(&OctaveOffset, 1, ac, av);
			//x->Features.SetOctaveOffset(OctaveOffset);
		}
	}
}

//	For any input
void markov_anything(t_markov *x, t_symbol *s, long ac, t_atom *av)
{
	switch (proxy_getinlet((t_object *)x))
	{
		case 0:
			if (s == gensym("resetTree"))
			{
				POST((t_object *)x, (char *)"Resetting Tree...");
				x->vpNotes.ResetViewpoint(2);
			}
			break;
			
		case 1:
			break;
			
		default:
			atom_setsym(&x->val, s);
			POST((t_object *)x, (char *)"Something received! ac=%ld; av=%ld",ac,av);
	}
}

//	Generate next note based on the model & current state upon receiving a bang
//	TODO: ...
int markov_bang(t_markov *x)
{
	int retval = 0;
	double Evaluation = 0;
	c1DVectorList ResultStream;
	tBundle2DHidden *PDF = new tBundle2DHidden();
	t2DNodeHidden* TempNode = NULL;
	c1DList * outcome = new c1DList();
	
	switch (proxy_getinlet((t_object *)x))
	{
		case 0:
			
			// A bang on the main inlet is a simple prediction.
			// Here's what happens:
			// 1. Based on the search stream, it navigates the tree using Viterbi Decoding
			// 2. Reaches the last state in the sequence
			// 3. Predicts the next state and then moves to this next state and predicts the next Symbol
			
			// If the next state does not exist, then it calls on the escape probability and reduces
			// the order of the search and repeats the process till it finds something.
			
			x->vpNotes.PredictBest(PDF, x);
			TempNode = PDF->RollState(x);
			TempNode->Emit(outcome, x);
			POST((t_object*)x, (char *)"Next Symbol: %f", outcome->GetValue(0));
			
			break;
			
		case 1:
			
			// A bang on the second inlet (the rightmost one)
			// "Evaluates" the search stream that has been fed in so far.
			
			// the final output is the probability that this stream could be
			// produced by the model
			
			Evaluation = x->vpNotes.EvaluateStream(x);
			
			break;
			
		case 2: 
			
			// A bang on the second inlet (second from right)
			// This is for "decoding" the current search stream
			// It uses the Viterbi algorithm to traverse the Trie
			// and returns the most likely sequence of states that led to 
			// the current search stream.
			
			x->vpNotes.DecodeStream(&ResultStream, x);
			break;
			
		case 3: // Bar input
			break;
			
		default:
			break;
	}
	return retval;
}

//	Identify
void markov_identify(t_markov *x)
{
	POST((t_object *)x, (char *)"My name is %s", x->name->s_name);
}

//	Object instantiation
void *markov_new(t_symbol *s, long argc, t_atom *argv)
{
	t_markov *x = NULL;

#ifdef	MAX4

	// object instantiation, OLD STYLE
	if (x = (t_markov *)newobject(markov_class))

#elif defined	MAX5

	// object instantiation, NEW STYLE
	if (x = (t_markov *)object_alloc((t_class *)markov_class))
		
#endif

	{
		x->name = gensym((char *)"");
		if (argc && argv) {
			x->name = atom_getsym(argv);
		}
		if (!x->name || x->name == gensym((char *)""))
			x->name = symbol_unique();
		
		atom_setlong(&x->val, 0);
		
		//	Instantiate inlets
		x->pIn2 = proxy_new(x, 1, &x->in);
		x->pInBeat = proxy_new(x, 2, &x->in);
		x->pInMeasure = proxy_new(x, 3, &x->in);
		
		//	Instantiate outlets from right to left
		x->pOutProb = outlet_new(x, NULL);
		x->pOutCount = outlet_new(x, NULL);
		x->pOutStream = outlet_new(x, NULL);
		x->pOut = outlet_new(x, NULL);
	}
	
	//	Call initialization method
	markov_init(x, s, argc, argv);

	return (x);
}

// Dummy function to get rid of the errors
unsigned int markov_beat( void* x) {
	return eRETVAL_SUCCESS;
}

//	Object initialization
void markov_init(t_markov *x, t_symbol *s, long ac, t_atom *av)
{
	POST((t_object *)x, (char *)"init - Before calling Gaussian");

	//CreateDummyTree(x);
	srand(time(NULL));	
}

void CreateDummyTree(t_markov* x)
{
	c1DList Vector(NULL,NULL,0);
	
	for(int i = 0; i < NUM_STROKES; i++)
	{
		// Vector = i
		Vector.Append((float)(i+1));
		
		// Create a basic level 1 trie using the gaussian data - mostly for testing
		//x->vpNotes.Trie.Append(&Vector, x->StrokePriors[i], &x->StrokeMeans[i][0], &x->StrokeVariance[0][0], &x->StrokeInverse[0][0], &x->StrokeDeterminant);
		
		Vector.DeleteAll();
	}
}

void PopulateGaussians(cViewPointHidden* ViewPoint, unsigned int SetNumber)
{
	static char Input1[10];
	char FilePath[150];
	float Number1 = 0;	
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int Mode = 0;
	
	sprintf(FilePath, "/Users/akito/Avinash/MaxSDK-5.1.1/examples/trimarkov_HMM/final/Training/set1/manualCovSharedPhrase%iOut.txt", SetNumber);
	
	ifstream FeatureFile;
	FeatureFile.open(FilePath);
	
	if (FeatureFile.is_open())
	{
		POST((t_object *)x, (char*)"---------Populating Gaussians: Run %d--------", SetNumber);
		POST((t_object *)x, (char*)"Reading File: %s", FilePath);
		
		while (!FeatureFile.eof()) {
			
			// This reads the file one "word" at a time
			FeatureFile >> Input1;
			
			if (strcmp("priors", Input1) == 0)
			{
				Mode = 1;
				i = 0;
				j = 0;
			}
			else if (strcmp("means", Input1) == 0)
			{
				Mode = 2;
				i = 0;
				j = 0;
			}
			else if (strcmp("covars", Input1) == 0)
			{
				Mode = 3;
				i = 0;
				j = 0;
			}
			else if (strcmp("invCovars", Input1) == 0)
			{
				Mode = 4;
				i = 0;
				j = 0;
			}
			else if (strcmp("detCovars", Input1) == 0)
			{
				Mode = 5;
				i = 0;
				j = 0;
			}
			else 
			{
				// This means it is a number
				// Convert to float
				Number1 = atof(Input1);

				switch (Mode) {
					case 1:
						// Set Priors
						//Assign the value
						ViewPoint->StrokePriors[i] = Number1;
						i++;
						break;
						
					case 2:
						// Set Means
						ViewPoint->StrokeMeans[j][i] = Number1;
						i++;
						
						if (i == NUM_FEATURES)
						{
							j++;
							i = 0;
						}
						break;
						
					case 3:
						// Set CoVars
						ViewPoint->StrokeVariance[j][i] = Number1;
						i++;
						
						if (i == NUM_FEATURES)
						{
							j++;
							i = 0;
						}
						break;
						
					case 4:
						// Set InvCoVars
						ViewPoint->StrokeInverse[j][i] = Number1;
						i++;
						
						if (i == NUM_FEATURES)
						{
							j++;
							i = 0;
						}
						break;
						
					case 5:
						// Set DetCovar
						ViewPoint->StrokeDeterminant = Number1;
						break;
					default:
						// Don't do anything if the mode is not set
						break;
				}
			}

		}
		
		// Print priors
		for (i = 0; i < NUM_STROKES; i++)
		{
			//POST((t_object*)x, "Prior[%d]: %f", i, ViewPoint->StrokePriors[i]);
		}
		
		// Print means
		for (j = 0; j < NUM_STROKES; j++)
		{
			for (i = 0; i < NUM_FEATURES; i++)
			{
				//POST((t_object*)x, "Mean[%d][%d]: %f",j, i, ViewPoint->StrokeMeans[j][i]);
			}
		}
		
		// Print variance
		for (j = 0; j < NUM_FEATURES; j++)
		{
			for (i = 0; i < NUM_FEATURES; i++)
			{
				//POST((t_object*)x, "Variance[%d][%d]: %f",j, i, ViewPoint->StrokeVariance[j][i]);
			}
		}
		
		// Print inverse
		for (j = 0; j < NUM_FEATURES; j++)
		{
			for (i = 0; i < NUM_FEATURES; i++)
			{
				//POST((t_object*)x, "Inverse[%d][%d]: %f",j, i, ViewPoint->StrokeInverse[j][i]);
			}
		}
		
		// Print Determinant
		//POST((t_object*)x, "Determinant: %f",ViewPoint->StrokeDeterminant);

		FeatureFile.close();
	}
}