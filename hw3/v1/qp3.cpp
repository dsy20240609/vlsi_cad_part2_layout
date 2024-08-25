#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include "solver.h"
#include <valarray>

using namespace std;

class netGatePair {
    public: 
	    netGatePair(int netId); //constructor
            
            //set method	    
	    void setGateId1(int id);
	    void setGateId2(int id);
	    void setIsGatePair(bool value);

	    //get method
	    int getGateId1() {return gateId1;}
	    int getGateId2() {return gateId2;}
            int getNetId()   {return netId;}

	    // member field
            int netId;
	    int gateId1;
	    int gateId2; 
	    // used for representing second gate the net connected to Or the Pad Id the net connected to.
	    bool isGatePair;
};

netGatePair::netGatePair(int netId) :netId(netId), 
	                             gateId1(0),
				     gateId2(0),
				     isGatePair(true) {}

void
netGatePair::setGateId1(int id) {
    this->gateId1 = id;
}

void
netGatePair::setGateId2(int id) {
    this->gateId2 = id;
}

void
netGatePair::setIsGatePair(bool value) {
    this->isGatePair=value;
}

int main (int argc, char* argv[]) {
//################### Parse the original Netlist and stored the gate net connection into netConnVec and padLoc.	
  if(argv[1] == "") {
    printf("Please provide the input netlist file.\n");
  }
  std::ifstream infile(argv[1]);
  int count=0;  //Count Value to record which line of the input file has been parsed
  int nGate=0;  //Number of Gate in the Netlist
  int nNet=0;   //Number of Nets in the Netlist
  int nPad=0;   //Number of Pads in the Netlist
  vector<pair<int,int>> padLoc;
  vector<netGatePair*>  netConnVec;
  if(infile.is_open()) {
    std::string line;
    while(std::getline(infile,line)) {
      std::vector<std::string> splits;
      std::stringstream ss(line);
      while(ss >> line) {
        splits.push_back(line);
      }
      if(count == 0) {
        if(splits.size() != 2) {
	  printf("input file format not correct!\n");
	} else {
	  nGate=atoi(splits[0].c_str());
	  nNet=atoi(splits[1].c_str()); 
	  netConnVec.resize(nNet,NULL);
	}
      } else if (count <= nGate) {
	  printf("line 74 count is %d\n", count);
	  printf("line 75 nGate is %d\n", nGate);
          if(splits.size() <4 ) {
	    printf("input file format for gate net netNumber is not correct!\n");
	  } else {
            int gateNumber = atoi(splits[0].c_str()); 
	    int numNets = atoi(splits[1].c_str());
	    for(int i = 2; i< splits.size();i++) {
	      if(netConnVec[atoi(splits[i].c_str())-1] == NULL) {
		  netGatePair * pair = new netGatePair(atoi(splits[i].c_str())-1);
		  pair->setGateId1(gateNumber);
		  netConnVec[atoi(splits[i].c_str())-1] = pair;
	      } else if(netConnVec[atoi(splits[i].c_str())-1]->gateId2==0) {
	          netConnVec[atoi(splits[i].c_str())-1]->setGateId2(gateNumber);
	      } // end if
	    } // end for  
	  }
      } else if (count == 1 + nGate) {
          if(splits.size() != 1) {
	    printf("input file format for pad number is not correct!\n");
	  } else {
	    nPad=atoi(splits[0].c_str()); 
	  }
      printf("line 96 padNumber is %d\n", nPad);
      } else {
          if(splits.size() != 4) {
	    printf("input file format for pad location is not correct!\n");
	  } else {
	    int padNumber=atoi(splits[0].c_str());
	    int netNumForPad = atoi(splits[1].c_str());
	    if(netNumForPad <= 0 || netNumForPad > netConnVec.size()) {
	      printf("Net %d doesn't not exist in the netlist\n", netNumForPad);
	    } else {
	      printf("line 106 %d\n",netNumForPad);
	      for(int i = 0 ; i < netConnVec.size(); i++) {
	        printf("line 108 %d %p\n",i,netConnVec[i]);
	      }
	      netConnVec[netNumForPad-1]->setGateId2(padNumber);
	      netConnVec[netNumForPad-1]->setIsGatePair(false);
	      printf("line 108\n");
	      int x = atoi(splits[2].c_str());
	      int y = atoi(splits[3].c_str());
	      padLoc.push_back(make_pair(x ,y));
	    }  
	  }
      } // end else
      count++; 
    } // end while
  }// end_if file open
printf("line 116 netConnVec size %d and padLoc size is %d\n", netConnVec.size(), padLoc.size());
//########### debug info to check the contents of netConnVec nad padLoc
for(int i = 0; i < netConnVec.size(); i++) {
  std::cout<< i << " " << " netId is " << netConnVec[i]->getNetId() << " gateId1 is " << netConnVec[i]->getGateId1() << " gateId2 is " << netConnVec[i]->getGateId2() << " is Gate or Pad " << netConnVec[i]->isGatePair<<endl;  
}
for(int i = 0 ; i < padLoc.size(); i++) {
  std::cout<<i << " x " << padLoc[i].first << " y is "<< padLoc[i].second<<endl;
}


//################## Next is to Build matrix C, matrix A and vector b from netConnVec and padLoc.
coo_matrix A;
int R[nNet];
int C[nNet];
double V[nNet];
valarray<double> bx(nGate);
valarray<double> by(nGate);
for(int i = 0 ; i < netConnVec.size(); i++) {
  R[i] = netConnVec[i]->getGateId1() -1;
  if(netConnVec[i]->isGatePair) {
    C[i] = netConnVec[i]->getGateId2() -1;
    V[i] = -1.0;
  } else { // net connect between gate and pad
    C[i]= netConnVec[i]->getGateId1() -1;
    int curRow=C[i];
    int digVal = 0;
    for(int j = 0; j < nNet; j++) {
      if(R[j] == curRow) {
        digVal++;
      }
    }
  V[i]=digVal;
  bx[netConnVec[i]->getGateId1()-1]=1.0*padLoc[netConnVec[i]->getGateId2()-1].first;
  by[netConnVec[i]->getGateId1()-1]=1.0*padLoc[netConnVec[i]->getGateId2()-1].second;
  }
}
A.n = nGate;
A.nnz = sizeof(R)/sizeof(int);
A.row.resize(A.nnz);
A.col.resize(A.nnz);
A.dat.resize(A.nnz);
A.row=valarray<int>(R, A.nnz);
A.col=valarray<int>(C, A.nnz);
A.dat=valarray<double>(V,A.nnz);
cout << "bx =  " << endl;
print_valarray(bx);
cout << "by = " << endl;
print_valarray(by);

valarray<double> x(A.n);
valarray<double> y(A.n);
A.solve(bx,x);
cout << "x = " << endl;
print_valarray(x);
A.solve(by,y);
cout << "y = " << endl;
print_valarray(y);
}
