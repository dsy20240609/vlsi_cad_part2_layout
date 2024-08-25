//VLSI CAD part 2 layout. Programming Assignment 3.
//Author: Shuyuan Du
//v1 previous v1 stored the gate<->net<->gate relationship wrongly. 
//v2 change the storage method into netId -> vector<gateId>, first build the netGateList to store a vector a gate that get connected through the same net. Ps. if there is a pad connnected through the net, always been put at the end of the vector, and the isGatePair flag is set to false.

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include "solver.h"
#include <valarray>

using namespace std;
// gateNetList used to store a netList the one gate has connect to. it is one gate versus mutiple nets that all connect to this gate.
class gateNetList {
  public : gateNetList(int gateId); // constructor
  void addNetToGate(int netId);  
  vector<int> nets;
  int gateId;
};
gateNetList::gateNetList(int gateId) : gateId(gateId),
	                               nets(vector<int>()) {}
				       
void				       
gateNetList::addNetToGate(int netId) {
  this->nets.push_back(netId);
} 
// netGateList used to store a gateList that one net connect. it is one net virsus mutiple gates that all get connected through the net.
class netGateList {
    public: 
	    netGateList(int netId); //constructor
            
            //set method
	    void addGateToNet(int gateId);	    
	    void setIsGatePair(bool value);

	    //get method
            int getNetId()   {return netId;}
            vector<int> getGateList() {return gateList;} 
	    // member field
            int netId;
	    vector<int> gateList;
	    // net connet between gate/gate or gate/pad
	    bool isGatePair;
};

netGateList::netGateList(int netId) :netId(netId),
                                     gateList(vector<int>()),	
				     isGatePair(true) {}

void
netGateList::addGateToNet(int gateId) {
    this->gateList.push_back(gateId);
}

void
netGateList::setIsGatePair(bool value) {
    this->isGatePair=value;
}

int main (int argc, char* argv[]) {
//part 1 Parse the original Netlist and stored the gate net connection into netConnVec, gateGonnVec and padLoc.	
  if(argv[1] == "") {
    printf("Please provide the input netlist file.\n");
  }
  std::ifstream infile(argv[1]);
  int count=0;  //Count Value to record which line of the input file has been parsed
  int nGate=0;  //Number of Gate in the Netlist
  int nNet=0;   //Number of Nets in the Netlist
  int nPad=0;   //Number of Pads in the Netlist
  vector<pair<int,int>> padLoc;
  vector<netGateList*>  netConnVec;
  vector<gateNetList*>  gateConnVec;
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
	  gateConnVec.resize(nGate,NULL);
	}
      } else if (count <= nGate) {
	  printf("line 74 count is %d\n", count);
	  printf("line 75 nGate is %d\n", nGate);
          if(splits.size() <4 ) {
	    printf("input file format for gate net netNumber is not correct!\n");
	  } else {
            int gateNumber = atoi(splits[0].c_str());
	    gateNetList *gatepair = new gateNetList(gateNumber-1);
	    int numNets = atoi(splits[1].c_str());
	    for(int i = 2; i< splits.size();i++) {
              gatepair->addNetToGate(atoi(splits[i].c_str())-1);		    
	      if(netConnVec[atoi(splits[i].c_str())-1] == NULL) {
		  netGateList * pair = new netGateList(atoi(splits[i].c_str())-1);
		  pair->addGateToNet(gateNumber);
		  netConnVec[atoi(splits[i].c_str())-1] = pair;
	      } else {
	          netConnVec[atoi(splits[i].c_str())-1]->addGateToNet(gateNumber);
	      } // end if
	    } // end for
	    gateConnVec[gateNumber-1]= gatepair;
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
	      netConnVec[netNumForPad-1]->addGateToNet(padNumber);
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
//debug info to check the contents of netConnVec nad padLoc
for(int i = 0; i < netConnVec.size(); i++) {
  std::cout<< i << " " << " netId is " << netConnVec[i]->getNetId() << " is Gate or Pad " << netConnVec[i]->isGatePair<<endl;  
  vector<int> gateList=netConnVec[i]->getGateList();
  std::cout << "connect gate is ";
  for(int j = 0 ; j < gateList.size(); j++) {
    std::cout << " " << gateList[j];
  }
  std::cout << endl;
}
for(int i = 0 ; i < gateConnVec.size(); i++) {
  std::cout<< i << " " << " gateId is " << gateConnVec[i]->gateId << endl;
  vector<int> netList=gateConnVec[i]->nets;
  std::cout << "connect net is ";
  for(int j = 0 ; j < netList.size(); j++) {
    std::cout << " "<< netList[j];
  }
  std::cout << endl; 
}

for(int i = 0 ; i < padLoc.size(); i++) {
  std::cout<<i << " x " << padLoc[i].first << " y is "<< padLoc[i].second<<endl;
}

//part 2 change netConnVec into R C V vector for matrix C.
//p --<5>-- 1 --<1>---2 
//               |
//	      <4>
//	       |
//	       |
//	       3
//c = 0 1 0   A = 6 -1 0 
//    1 0 4      -1 0 -4
//    0 4 0      0 -4  0
//element in matrix c is gate i and gate j connected through net m which, has a weight of c[i][j]
//element in matrix a is if not in diagonal, a[i][j] = -c[i][j], 
vector<int> R;
vector<int> C;
vector<double> V;
for(int i =0; i < netConnVec.size(); i++) {
  vector<int> gateList=netConnVec[i]->getGateList();
  int k = gateList.size();
  float weight = 1.0/(k-1);
  int gateNum = 0;
  if(netConnVec[i]->isGatePair) {
    gateNum=gateList.size()-1;
  } else {
    gateNum=gateList.size();
  }    
  for(int m = 0 ; m < gateNum; m++) {
    for (int n = 0 ; n < gateNum; n++) {
      if(m==n) {
        continue;
      } else {
        R.push_back(gateList[m]);
        C.push_back(gateList[n]);
        V.push_back((-1.0)*weight);
      }
    }
  }
}
printf("line 208\n");
//part 3 change matrix C into matrix A.
vector<double> gateValue(nGate,0.0);
for(int i = 0 ; i < R.size(); i++) {
  gateValue[R[i]]=gateValue[R[i]]+(-1.0)*V[R[i]];
}
printf("line 214\n");
for (int i = 0 ; i <nGate; i++) {
  vector<int> netList=gateConnVec[i]->nets;
  for(int j = 0 ; j < netList.size(); j++) {
    if(netConnVec[netList[j]]->isGatePair) {
      continue;
    } else {
      gateValue[i] = gateValue[i] + (1.0/(netConnVec[netList[j]]->gateList.size()-1));
    }
  }
}
printf("line 224\n");

for(int i = 0 ; i < nGate;i++) {
    R.push_back(i);
    C.push_back(i);
    V.push_back(gateValue[i]);
}

printf("line 231\n");

//part 4 get matrix b
//if gate i connects to a pad at (xi, yi) with a wire with weight wi
valarray<double> bx(0.0, nGate);
valarray<double> by(0.0, nGate);
for(int i = 0 ; i < netConnVec.size(); i++) {
  if (!netConnVec[i]->isGatePair) {
    int gateLength = netConnVec[i]->getGateList().size();
printf("line 241 %d %d\n",gateLength, netConnVec[i]->getGateList()[gateLength-1]);    
    double weight = 1.0/gateLength;
    printf("243 weight is %f\n", weight);
    int padIndex = netConnVec[i]->getGateList()[gateLength-1];
    for(int j=0; j < gateLength-1; j++) {
      bx[netConnVec[i]->getGateList()[j]]=weight*padLoc[padIndex-1].first;
      printf("line 247 %d %f\n", netConnVec[i]->getGateList()[j], weight*(padLoc[padIndex-1].first));
      by[netConnVec[i]->getGateList()[j]]=weight*padLoc[padIndex-1].second;
      printf("line 247 %d %f\n", netConnVec[i]->getGateList()[j], weight*(padLoc[padIndex-1].second));
//printf("line 247 %d %d\n", netConnVec[i]->getGateList().size(), padLoc.size());
    }
  }
}
printf("line 244\n");
printf("line 246\n");
for(auto ptr : netConnVec) {
  delete ptr;
}
printf("line 250\n");
netConnVec.clear();
printf("line 252\n");
for(auto ptr : gateConnVec) {
  delete ptr;
}
printf("line 256\n");
gateConnVec.clear();
printf("line 258\n");
coo_matrix A;
int R1[R.size()];
ofstream outfile;
outfile.open("debug.out", ios::out | ios::app);
for(int i = 0 ; i< R.size(); i++) {
R1[i]=R[i];
}
int C1[C.size()];
for(int i = 0 ; i <C.size(); i++) {
C1[i]=C[i];
}
double V1[V.size()];
for(int i = 0 ; i <V.size(); i++) {
V1[i]=V[i];
}
for(int i = 0 ; i < R.size(); i++) {
outfile<< R1[i] << " " << C1[i] << " " << V1[i] << endl;
}
outfile.close();

A.n = nGate;
A.nnz= sizeof(R1)/sizeof(int);
A.row.resize(A.nnz);
A.col.resize(A.nnz);
A.dat.resize(A.nnz);
A.row=valarray<int>(R1, A.nnz);
A.col=valarray<int>(C1, A.nnz);
A.dat=valarray<double>(V1,A.nnz);
cout << "bx =  " << endl;
print_valarray(bx);
cout << "by =  " << endl;
print_valarray(by);
ofstream outfile1;
outfile1.open("debug.out1", ios::out | ios::app);
for(int i = 0 ; i < bx.size(); i++) {
outfile1 << bx[i] << " " << by[i] << endl; 
}

valarray<double> x(A.n);
valarray<double> y(A.n);
A.solve(bx,x);
cout << "x = " << endl;
print_valarray(x);
A.solve(by,y);
cout << "y = " << endl;
print_valarray(y);
}
// part 5 use solver to solve the matrix.

//A.n = nGate;
//A.nnz = sizeof(R)/sizeof(int);
//A.row.resize(A.nnz);
//A.col.resize(A.nnz);
//A.dat.resize(A.nnz);
//A.row=valarray<int>(R, A.nnz);
//A.col=valarray<int>(C, A.nnz);
//A.dat=valarray<double>(V,A.nnz);
//cout << "bx =  " << endl;
//print_valarray(bx);
//cout << "by = " << endl;
//print_valarray(by);
//
//valarray<double> x(A.n);
//valarray<double> y(A.n);
//A.solve(bx,x);
//cout << "x = " << endl;
//print_valarray(x);
//A.solve(by,y);
//cout << "y = " << endl;
//print_valarray(y);
//}
