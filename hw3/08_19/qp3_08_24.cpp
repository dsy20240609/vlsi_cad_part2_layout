// 08/20/2024 new resolution
// part 1: transfer file input into matrix C
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <valarray>
#include "solver.h"
#include <cmath>

int main (int argc, char* argv[]) {
    if(argv[1] == "") {
        printf("Please provide the input netlist file.\n");
    }
    std::ifstream infile(argv[1]);
    int gateNumber=0;
    int netNumber=0;
    int count=0;
    vector<vector<int>> netGateVec;

    if(infile.is_open()) {
        std::string line;
	while(std::getline(infile,line)) {
	    std::vector<std::string> splits;
	    std::stringstream ss(line);
	    while(ss >> line) {
	        splits.push_back(line);
	    }
            if(splits.size() > 0) {
	        if(count ==0) {
	            if(splits.size() !=2) {
	                printf("input file format not correct\n");
	            } else {	
	              gateNumber=atoi(splits[0].c_str());
	              netNumber=atoi(splits[1].c_str());
	              netGateVec.resize(netNumber);
	            }
	            count++;
	        } else if (count <= gateNumber) { // gate part
	            int netConnectNumber=atoi(splits[1].c_str());    
	            for(size_t i = 2; i < splits.size() ;++i) {
	                netGateVec[atoi(splits[i].c_str())-1].emplace_back(atoi(splits[0].c_str()));    
	            }
	            count++;
	        } else if (count == gateNumber+1) {// port number
	            count++;
                } else { // port information
                    netGateVec[atoi(splits[1].c_str())-1].emplace_back(-1);
                    netGateVec[atoi(splits[1].c_str())-1].emplace_back(atoi(splits[0].c_str()));
                    netGateVec[atoi(splits[1].c_str())-1].emplace_back(atoi(splits[2].c_str()));
                    netGateVec[atoi(splits[1].c_str())-1].emplace_back(atoi(splits[3].c_str()));
                    count++;
	        }
	    }
	}
    }

// part 2: transfer matrix C to matrix A
   double matrixC[gateNumber][gateNumber]={0.0};
   for(size_t i = 0 ; i < netGateVec.size();++i) {
       vector<int> gates = netGateVec[i];
       vector<int>::iterator it=std::find(gates.begin(),gates.end(),-1);
       int gateNum=0;
       if (it == gates.end()) {
           gateNum = gates.size();
       } else {
           gateNum = it-gates.begin();
       }
       for(size_t j = 0 ; j < gateNum-1; ++j) {
           for(size_t k = j+1; k < gateNum; ++k) {
	       matrixC[gates[j]-1][gates[k]-1] += (-1.0)*1.0;
	       matrixC[gates[k]-1][gates[j]-1] += (-1.0)*1.0;
	       printf("line 134 %f %f\n", matrixC[gates[j]-1][gates[k]-1], matrixC[gates[k]-1][gates[j]-1]);
	   } 
       }
   }



   for(size_t i = 0 ; i < netGateVec.size();++i) {
       vector<int> gates = netGateVec[i];
       vector<int>::iterator it=std::find(gates.begin(),gates.end(),-1);
       if(it != gates.end()) {
           vector<int> padGates(gates.begin(),it);
	   for(size_t j = 0 ; j < padGates.size(); ++j) {
	       double sum=0.0;
	       for(size_t k = 0; k < gateNumber;k++) {
	           sum+= matrixC[padGates[j]-1][k];
	       } 
	       sum=sum*(-1.0) + 1.0;
	       matrixC[padGates[j]-1][padGates[j]-1]=matrixC[padGates[j]-1][padGates[j]-1]+sum;
	       printf("line 162 %f \n",matrixC[padGates[j]-1][padGates[j]-1]);
	   } 
       }    
   }
   ofstream outfile;
   outfile.open("debug", ios::out| ios::app);
   for(size_t i = 0 ; i < gateNumber; ++i) {
       for(size_t j = 0 ; j < gateNumber;++j) {
           outfile <<  matrixC[i][j] << " ";
       }
       outfile << endl;
   }

 // part 3 : write a row/column/r value from matrix A into a file
   vector<int> row;
   vector<int> column;
   vector<double> value;
   for(size_t  i=0; i < gateNumber; ++i) {
       for(size_t j =0; j < gateNumber; ++j) {
           if(!(fabs(matrixC[i][j]) < 1e-15)) {
             row.push_back(i);
             column.push_back(j);
             value.push_back(matrixC[i][j]);
           }    
       }
   }
   for(size_t i = 0 ; i < row.size();++i) {
       outfile << row[i] << " "<< column[i] << " " << value[i] << endl; 
   }
   outfile.close();
   int row1[row.size()];
   int column1[column.size()];
   double value1[value.size()];
   for(size_t i=0; i < row.size();++i) {
       row1[i] = row[i];
   }
   for(size_t i=0; i < column.size();++i) {
       column1[i] = row[i];
   }
   for(size_t i=0; i < value.size();++i) {
       value1[i] = value[i];
   }
// //part 4 : build Bx vector
    valarray<double> Bx(gateNumber);
    valarray<double> By(gateNumber);
    for(size_t i = 0 ; i < netGateVec.size();++i) {
        vector<int> gates=netGateVec[i];
        vector<int>::iterator it= std::find(gates.begin(),gates.end(),-1);
        if(it != gates.end()) {
          vector<int> padGates(gates.begin(),it);
          int x = gates[gates.size()-2];
          int y = gates[gates.size()-1];
          for(size_t j = 0; j < padGates.size(); ++j) {
              Bx[padGates[j]-1]=Bx[padGates[j]-1]+1.0*x;
              By[padGates[j]-1]=By[padGates[j]-1]+1.0*y; 
          }  
        }
    }
// call solver to solve placement matrix
    coo_matrix A;
    A.n=gateNumber;
    A.nnz=row.size();
    A.row.resize(A.nnz);
    A.col.resize(A.nnz);
    A.dat.resize(A.nnz);
    A.row = valarray<int>(row1, A.nnz);
    A.col=valarray<int>(column1,A.nnz);
    A.dat = valarray<double>(value1,A.nnz);
    valarray<double> x(A.n);
    valarray<double> y(A.n);
    printf("_________________\n");
    print_valarray(A.row);
    printf("__________________\n");
    print_valarray(A.col);
    printf("___________________\n");
    print_valarray(A.dat);
    print_valarray(Bx);
    printf("_______________\n");
    print_valarray(By);
    printf("_______________\n");
    A.solve(Bx,x);
    print_valarray(x);
    A.solve(By,y);
    print_valarray(y);
}




