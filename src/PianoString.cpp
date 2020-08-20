/*
* "Piano string" implementation based on a digigal waveguide
* with added filters for secondary effects.
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/

#include <math.h>
#include <string>

#include "NoteUtil.h"

#include "PianoString.h"

#define max(x,y) ( \
    { __auto_type __x = (x); __auto_type __y = (y); \
      __x > __y ? __x : __y; })

// -------------------------------------- Node -------------------------------------------

enum Rail {
	TOP= 0,
	BOTTOM= 1
};

// state at a delayline/waveguide boundary
class Node {
public:
	Node()								{ init(0); }
	
	void init(float z);
	
	float z()							{ return _z; }
	void forceLoad(float load)			{ this->_forceLoad= load;}
	float getForcedLoad()				{ return _forceLoad; }

	void setData(Rail r, float data)	{ _data[r]= data; }
	float getData(Rail r)				{ return _data[r]; }

private:
	// delay line cells
	float _data[2];		// top and bottom rail data
	
	// feed specific input into the rail
	float _forceLoad;

	// coupling of different waveguide segments
	// (load/velocity=impedance)
	float _z;			// impedance
};

void Node::init(float z) {
  
  _z = z;
  _forceLoad = 0;
  
  _data[TOP]= 0;
  _data[BOTTOM]= 0;
}


// -------------------------------------- NodeLink -------------------------------------------

enum LinkMode {
	SAME_RAIL= 0,
	CROSS_RAIL= 1
};

class NodeLink {
public: 
	NodeLink(std::string id, Node *n, LinkMode m, bool cleanup= false);
	~NodeLink() 						{ if (_cleanupMem) delete(_node); };
	
	void init(float z)					{ _node->init(z); }
		
	// handle links between nodes with different impedances
	float z()							{ return _node->z(); }
	float getForcedLoad()				{ return _node->getForcedLoad(); }
	float getAllotedLoad(Rail railIdx);
	void setSumZ(float zSum);
	
	Node* node()						{ return _node; }
	
	const char* getId()					{ return _dbgId.c_str(); }
private:
	LinkMode	_mode;		// cross-rail access handling
	Node*		_node;
	bool		_cleanupMem;	// mem management

	std::string	_dbgId;
	
	float		_allotment;	// weight of the respective linked node
};
	
NodeLink::NodeLink(std::string id, Node *n, LinkMode m, bool cleanup) {
	_dbgId= id;
	_node= n;
	_mode= m;
	_cleanupMem= cleanup;
}

float NodeLink::getAllotedLoad(Rail railIdx) {
	return _allotment * _node->getData( (Rail)(_mode ^ railIdx));
}

static float sSumZC= 2;
void NodeLink::setSumZ(float zSum) {
	_allotment= sSumZC * z() / zSum;
}


// -------------------------------------- Waveguide -------------------------------------------

/*
* Digital waveguide implementation with provisions for chaining and handling
* of add-on filters.
*/
class Waveguide {
public:
	Waveguide(std::string id, PianoString *string,
				float (*topFunc)(PianoString*, float)= NULL , float (bottomFunc)(PianoString*, float)= NULL,
				bool propagateLeft= true, bool propagateRight= true );
	~Waveguide();

	/*
	* @param z impedance of the string
	* @param topDel delay for top rail
	* @param bottomDel delay for bottom rail
	*/
	void initBegin(float z, int topDel, int bottomDel);
	void initEnd();	// all initBegin must be completed 1st

	void addLinkLeft(Node *n, LinkMode m);
	void addLinkRight(Node *n, LinkMode m);
	
	// basic delayline handling
	void shiftDelaylines();
	void handleBorderReflection();

	// accessors for direct manipulation
	Node *rightNode()					{ return _right->node(); }
	Node *leftNode()					{ return _left->node(); }

	void setRight(Rail r, float data)	{ _right->node()->setData(r, data); }
	float getRight(Rail r)				{ return _right->node()->getData(r); }

	void setLeft(Rail r, float data)	{ _left->node()->setData(r, data); }
	float getLeft(Rail r)				{ return _left->node()->getData(r); }
	
	float z()							{ return _z; }
private:
	float getTotalLoad(NodeLink *node, std::vector<NodeLink*> *links, Rail railIdx);
	void setSumZ(NodeLink *node, std::vector<NodeLink*> *links);
private:
	PianoString*			_string;
	
	float					_z;
	
	// left side
	NodeLink*				_left;
	std::vector<NodeLink*>	_leftLinks;
	bool					_propagateLeft;

	// right side
	NodeLink*				_right;
	std::vector<NodeLink*>	_rightLinks;
	bool					_propagateRight;
	
	// top/bottom rail related
	DelayFilter 			_delayFilterBottom;
	DelayFilter 			_delayFilterTop;
	
	float (*_railFeedFunc[2])(PianoString*, float);
};


float passThrough(PianoString *s, float in) {
	return in;	// default delay line impl
}

Waveguide::Waveguide(std::string id, PianoString *string,
		float (*topFunc)(PianoString*, float), float (bottomFunc)(PianoString*, float),
		bool propagateLeft, bool propagateRight ) {
			
	_string= string;
	
	_propagateLeft= propagateLeft;
	_propagateRight= propagateRight;

	_railFeedFunc[TOP]=		(topFunc == NULL)	? passThrough : topFunc;
	_railFeedFunc[BOTTOM]=	(bottomFunc == NULL)? passThrough : bottomFunc;

	_left=	new NodeLink(id+"-L", new Node(), SAME_RAIL, true);
	_right=	new NodeLink(id+"-R", new Node(), SAME_RAIL, true);
}

Waveguide::~Waveguide(){
	delete _left;
	delete _right;
	
	for (int i= 0; i<_leftLinks.size(); i++) {
		delete(_leftLinks[i]);
	}
	for (int i= 0; i<_rightLinks.size(); i++) {
		delete(_rightLinks[i]);
	}
}

void Waveguide::initBegin(float z, int topDel, int bottomDel) {
	_z= z;
	
	// todo: without -1 it sounds like a detuned saloon piano.. -> investigate
	_delayFilterTop.init(topDel-1);	
	_delayFilterBottom.init(bottomDel-1);
		
	_left->init(z);
	_right->init(z);
}

void Waveguide::setSumZ(NodeLink *node, std::vector<NodeLink*> *links) {
	// total impedance
	float sumZ = node->z();
	for(int i=0; i<links->size(); i++) {
		sumZ += (*links)[i]->z();
	}
	// cache precalculated weights
	node->setSumZ(sumZ);
	for(int i=0; i<links->size(); i++) {
		(*links)[i]->setSumZ(sumZ);
	}
}
	
void Waveguide::initEnd() {
	setSumZ(_left, &_leftLinks);
	setSumZ(_right, &_rightLinks);
}

void Waveguide::addLinkLeft(Node *l, LinkMode m) {
	_leftLinks.push_back(new NodeLink("l", l, m));
}

void Waveguide::addLinkRight(Node *r, LinkMode m) {
	_rightLinks.push_back(new NodeLink("r", r, m));
}

float Waveguide::getTotalLoad(NodeLink *node, std::vector<NodeLink*> *links, Rail railIdx) {
	
	float load= node->getForcedLoad();
	load+= node->getAllotedLoad(railIdx);
	
	for (int i= 0; i<links->size(); i++) {
		load+= (*links)[i]->getForcedLoad();		
		load+= (*links)[i]->getAllotedLoad(railIdx);
	}
	return load;
}

static bool sForcePropagate= false;

void Waveguide::handleBorderReflection() {
	// handle reflection of the signal at the borders of the waveguide
	
	float leftLoad=	 !(_propagateLeft || sForcePropagate) ? 0 : getTotalLoad(_left, &_leftLinks, TOP);
	float rightLoad= !(_propagateRight || sForcePropagate) ? 0 : getTotalLoad(_right, &_rightLinks, BOTTOM);
	
	float vl = (leftLoad - getLeft(TOP));			// top to bottom
	setLeft(BOTTOM, _railFeedFunc[TOP](_string, vl));

	float vr = (rightLoad - getRight(BOTTOM));		// bottom to top
	setRight(TOP, _railFeedFunc[BOTTOM](_string, vr));
}

void Waveguide::shiftDelaylines() {
		
	// right to left
	setLeft(TOP, _delayFilterTop.process(getRight(TOP)));
	
	// left to right
	setRight(BOTTOM, _delayFilterBottom.process(getLeft(BOTTOM)));
}


// ----------------------------------- PianoString -------------------------------------------

float tuningDelay(PianoString *s, float in) {
	// add tuning delay somewhere into the round trip
	return s->_fracDelayFilter.process(in);
}
float leftReflect(PianoString *s, float in) {
	return s->_dispersionFilter.process(in);
}
float rightReflect(PianoString *s, float in) {
	return s->_lossFilter.process(in);
}

PianoString::PianoString(float sampleRate) {
	_sampleRate= sampleRate;

	// string segments
	_stringS0= new Waveguide("S0", this, tuningDelay,	NULL, 			false);
	_stringS1= new Waveguide("S1", this, leftReflect,	rightReflect, 	true, false);
	_stringS0->addLinkRight(_stringS1->leftNode(), CROSS_RAIL);
	_stringS1->addLinkLeft(_stringS0->rightNode(), CROSS_RAIL);
	
	_hammerNode= new Node();	
	_stringS0->addLinkRight(_hammerNode, CROSS_RAIL);	// feed hammer into both rails
	_stringS1->addLinkLeft(_hammerNode, SAME_RAIL);
}

PianoString::~PianoString() {
	delete(_stringS0);
	delete(_stringS1);
	delete(_hammerNode);
}

static float sDetune= 7;

void PianoString::setStringTweaks(float sumZ, float detune, bool propagate) {
	sSumZC= sumZ;
	sDetune= detune;
	sForcePropagate= propagate;
}

float delay(float d) {
	return max(0.0, floor(d));
}

void PianoString::initLossDispersion(float freq, float b) {
	_dispersionFilter.init(freq, b);
	
	float c1 = 0.5;	// see page 119 - some tuning might be in order here
	float c3 = 13.85;
	_lossFilter.init(freq, c1, c3);	// 1st order low-pass filter
}
	
float PianoString::initStringSegments(float freq, int midiNote, float z, float t) {
	// setup timing of the different waveguide segments
	
	// the relative position of the hammer's strike determines the harmonics that will
	// be eliminated.. e.g. at 1/5 it would be the 5th, 10th, 15st, etc => string
	// waveguides timing is split according to the "striking point".
	
	// todo: use more realistic striking point: "For approximately 60 upper notes of
	// the grand piano, the position of the striking point determined by parameter
	// 'hp' gradually becomes displaced from 1/8 to 1/24 of the whole string length in
	// the direction of the high notes"

	float linPos= 	getLinearPos(midiNote);

	float hp= 1.0/(7.0+linPos*17);	// "striking point" vgl. s.20 https://ccrma.stanford.edu/~jos/pmudw/pmudw.pdf		

	float sumDelay= 0;		
	float t2= t/2;		// total samples for one direction
	
	
	// "string segment 1" (symetric for both directions)
		
	int s1 = delay(hp * t2);
	sumDelay+= 2*s1;

	_stringS0->initBegin(z, s1, s1);	// waveguide 1

	
	// "string segment 2" (one direction handles the "dispersion" the other the "loss")

	float dispersion= _dispersionFilter.getGroupDelay(freq, _sampleRate);
	sumDelay+= dispersion;

	float loss= _lossFilter.getGroupDelay(freq, _sampleRate);
	sumDelay+= loss;

	int s2l= delay( ((1-hp) * t2) - dispersion);
	int s2r= delay( ((1-hp) * t2) - loss);
	sumDelay+= s2l+s2r;

	_stringS1->initBegin(z, s2l, s2r);	// waveguide 2
	
	return sumDelay;
}

void PianoString::init(float freq, int midiNote, float b, float z, float zb, float zh, float tweak) {
	_2Z= z*2;
	_iZb= 1.0/ zb; 			// avoid "expensive" divisions later
	_i2Z= 1.0/ (2.0*z);
	
	initLossDispersion(freq, b);

	//"per key.. , there should be at least two filtered delay loops, tuned differently.."
	float t= _sampleRate/freq* pow(tweak, sDetune);	// total samples for both directions

	float sumDelay= initStringSegments(freq, midiNote, z, t);
	
	// Jaffe and Smith:  "The fact that the delay-line length N must be an integer causes tuning
	// problems. Since the fundamental frequency is f, = f/(N + 1/2)" => tuning: "an additional
	// fractional delay Thiran allpass filter must be inserted to tune the overall delay"
		
	float requiredTuning= (t - sumDelay);
	_fracDelayFilter.init((int)requiredTuning, requiredTuning);
		
	// complete the waveguide setup
	
	_hammerNode->init(zh);
	
	_stringS0->initEnd();
	_stringS1->initEnd();
}

float PianoString::getStringVelocity() {
	// get string's velocity at the hammer's "striking
	// point", i.e. between the two string segments
	
	return	_stringS1->getLeft(TOP) + 
			_stringS0->getRight(BOTTOM);
}

float PianoString::getBridgeSignal(float fin) {
	// "impedance [is] ratio of force to velocity at the driving point"
	float vin= fin *_i2Z;

	// "The excitation force can be taken into account by adding vin = 
	// Fin/(2*Z0) to both delay lines at the position of the excitation"
	_hammerNode->forceLoad(vin);
	
	// " After that, the delay lines are shifted"
	_stringS0->shiftDelaylines();
	_stringS1->shiftDelaylines();

	// calc signal at the soundboard's bridge
	float out= _stringS1->getRight(BOTTOM) *_iZb * _2Z;
	
	_stringS0->handleBorderReflection();
	_stringS1->handleBorderReflection();
	
	return out;
}

