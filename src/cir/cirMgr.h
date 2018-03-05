/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;
class fecGrp;

class CirMgr
{
public:
   CirMgr() { _isSweep=false; _dfsColor=false; }
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid);

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist();
   void printPIs() const;
   void printPOs() const;
   void printFloatGates();
   void printFECPairs() const;
   void writeAag(ostream&);
   void writeGate(ostream&, CirGate*);

   vector<fecGrp*> _fecGrps;

private:
   ofstream           *_simLog;
   vector<CirGate *> _piList;
   vector<CirGate *> _poList;
   vector<CirGate *> _dfsList;
   //vector<CirGate *> _idfsList;
   map<unsigned,CirGate *> _totalList;
   unsigned M,I,L,O,A;
   bool _isSweep;
   bool _dfsColor;
   SatSolver* _solver;

   CirGate* findOrCreate(unsigned);
   void updateDfslist();
   void doDfs(CirGate *);
   void doDfs2(CirGate *,vector<CirGate*>*);
   unsigned outputNetlist(ostream&, bool);
   void resetColor();
   void resetColor2();
   void replace(CirGate*,CirGate*);
   void replaceInv(CirGate*,CirGate*);
   void separateFECs();
};

#endif // CIR_MGR_H
