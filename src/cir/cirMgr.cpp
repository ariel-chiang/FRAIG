/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   char *file;						//open file
   file=new char[fileName.size()+1];
   strcpy(file,fileName.c_str());
   ifstream fin(file);
   delete [] file;
   if(!fin.is_open()) {
      cout<<"Cannot open disign \""<<fileName<<"\"!!"<<endl;
      return false;
   }

   string line;
   string token;
   int k;
   fin>>line;
   fin>>M>>I>>L>>O>>A;
   getline(fin,line);

   for( unsigned i=1 ; i<=I ; ++i ) {
      if(getline(fin,line)) {
         myStrGetTok(line,token);
         myStr2Int(token,k);
         CirGate* g=new CirGate((unsigned)k/2,i+1,1,PI);
         _piList.push_back(g);
         _totalList[(unsigned)k/2]=g;
      }
   }

   for( unsigned i=1 ; i<=O ; ++i ) {
      if(getline(fin,line)) {
         myStrGetTok(line,token);
         myStr2Int(token,k);
         CirGate* g=findOrCreate((unsigned)k/2);
         CirGate* po=new CirGate(M+i,i+I+1,1,PO);
         _poList.push_back(po);
         _totalList[(unsigned)k/2]=g;

         g->addFanout(po);
         po->addFanin(g,k%2==1);
      }
   }

   for( unsigned i=1 ; i<=A ; ++i ) {
      if(getline(fin,line)) {
         int pos=myStrGetTok(line,token); myStr2Int(token,k);
         CirGate* g=findOrCreate((unsigned)k/2);
         if(g->getLineNo()==0) {
            g->setLine(I+O+i+1); g->setCol(1); g->setType(AIG);
         }

         pos=myStrGetTok(line,token,pos); myStr2Int(token,k);
         CirGate* ig=findOrCreate((unsigned)k/2);
         g->addFanin(ig,k%2==1);
//if(k%2==1) cout<<g->getGateId()<<" has invert"<<endl;
         ig->addFanout(g);

         pos=myStrGetTok(line,token,pos); myStr2Int(token,k);
         ig=findOrCreate((unsigned)k/2);
         g->addFanin(ig,k%2==1);
//if(k%2==1) cout<<g->getGateId()<<" has invert"<<endl;
         ig->addFanout(g);
      }
   }

   while(getline(fin,line) && line!="c") {
      if( line[0]=='i' ) {
         int pos=myStrGetTok(line,token,1);
         if( myStr2Int(token,k) ) {
            if( (unsigned)k<_piList.size() ) {
               myStrGetTok(line,token,pos);
               _piList[k]->setName(token);
            }
         }
      }
      else if( line[0]=='o' ) {
         int pos=myStrGetTok(line,token,1);
         if( myStr2Int(token,k) ) {
            if( (unsigned)k<_poList.size() ) {
               myStrGetTok(line,token,pos);
               _poList[k]->setName(token);
            }
         }
      }
   }

   fin.close();
   return true;
}

CirGate*
CirMgr::getGate(unsigned gid)
{
   map<unsigned,CirGate*>::iterator i;
   i=_totalList.find(gid);
   if( i!=_totalList.end() ) return i->second;
   for( unsigned j=0 ; j<_poList.size() ; ++j )
      if( _poList[j]->getGateId()==gid )
         return _poList[j];
   return NULL;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout<<endl<<"Circuit Statistics"<<endl;
   cout<<"=================="<<endl;
   cout<<"  PI"<<setw(12)<<I<<endl;
   cout<<"  PO"<<setw(12)<<O<<endl;
   cout<<"  AIG"<<setw(11)<<A<<endl;
   cout<<"------------------"<<endl;
   cout<<"  Total"<<setw(9)<<I+O+A<<endl;
}

void
CirMgr::printNetlist()
{
   bool flag=_dfsColor;
   findOrCreate(0)->_color=_dfsColor;
   unsigned count=0;
   cout<<endl;
   for( unsigned i=0 ; i<_poList.size() ; ++i ) {
      _poList[i]->printFanin(flag,count);
   }
   _dfsColor=!_dfsColor;
}

unsigned
CirMgr::outputNetlist(ostream& outfile, bool toPrint)
{
   bool flag=_dfsColor;
   findOrCreate(0)->_color=_dfsColor;
   unsigned count=0;
   for( unsigned i=0 ; i<_poList.size() ; ++i ) {
      _poList[i]->outputFanin(flag,outfile,count,toPrint);
   }
   _dfsColor=!_dfsColor;
   return count;
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for( unsigned i=0 ; i<_piList.size() ; ++i )
      cout<<" "<<_piList[i]->getGateId();
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for( unsigned i=0 ; i<_poList.size() ; ++i )
      cout<<" "<<_poList[i]->getGateId();
   cout << endl;
}

void
CirMgr::printFloatGates()
{
   bool flag=false;
   map<unsigned,CirGate*>::iterator i;
   for( i=_totalList.begin() ; i!=_totalList.end() ; ++i ) {
      if( i->second->getType() == Undef ) {
         if(!flag) { cout<<"Gates with floating fanin(s):"; flag=true; }
         cout<<" "<<i->first;
      }
   }
   if(flag) { cout<<endl; flag=false; }

   for( i=_totalList.begin() ; i!=_totalList.end() ; ++i ) {
      if( i->second->getType() != PO && i->second->getFanoutId()==0 ) {
         if(!flag) { cout<<"Gates defined but not used  :"; flag=true; }
         cout<<" "<<i->first;
      }
   }
   if(flag) cout<<endl;
}

void
CirMgr::writeAag(ostream& outfile)
{
   outfile<<"aag "<<M<<" "<<I<<" "<<L<<" "<<O<<" "<<outputNetlist(outfile,false)<<endl;
   for( unsigned i=0 ; i<_piList.size() ; ++i )
      outfile<<_piList[i]->getGateId()*2<<endl;
   for( unsigned i=0 ; i<_poList.size() ; ++i )
      outfile<<_poList[i]->getFaninId1()<<endl;
   outputNetlist(outfile,true);
   for( unsigned i=0 ; i<_piList.size() ; ++i )
      if( !_piList[i]->getName().empty() )
         outfile<<"i"<<i<<" "<<_piList[i]->getName()<<endl;
   for( unsigned i=0 ; i<_poList.size() ; ++i )
      if( !_poList[i]->getName().empty() )
         outfile<<"o"<<i<<" "<<_poList[i]->getName()<<endl;
   outfile<<"c"<<endl;
   outfile<<"AAG output by Tsai-Jong Chiang";
}

CirGate*
CirMgr::findOrCreate(unsigned id)
{
   map<unsigned,CirGate*>::iterator i;
   i=_totalList.find(id);
   if(i!=_totalList.end())
      return i->second;
   CirGate* g;
   if( id==0 ) g=new CirGate(id,0,0,Const);
   else g=new CirGate(id,0,0);
   _totalList[id]=g;
   return g;
}

void CirMgr::resetColor() {
      map<unsigned,CirGate*>::iterator i;
      for( i=_totalList.begin() ; i!=_totalList.end() ; ++i )
         i->second->setColor(false);
      for( unsigned j=0 ; j<_poList.size() ; ++j )
         _poList[j]->setColor(false);
      _dfsColor=false;
   }

void CirMgr::resetColor2() {
      map<unsigned,CirGate*>::iterator i;
      for( i=_totalList.begin() ; i!=_totalList.end() ; ++i )
         i->second->setColor2(false);
      for( unsigned j=0 ; j<_poList.size() ; ++j )
         _poList[j]->setColor2(false);
   }

void
CirMgr::updateDfslist()
{
   resetColor();
   _dfsList.clear();
   for( unsigned i=0 ; i<_poList.size() ; ++i )
      doDfs(_poList[i]);
   _dfsColor=true;
}

void
CirMgr::doDfs(CirGate* g)
{
   if( g->getColor()==false ) {
      g->setColor(true);
      switch(g->getType()) {
      case 2: doDfs(g->getFanin0());
              doDfs(g->getFanin1());
              _dfsList.push_back(g);
              break;
      case 1: doDfs(g->getFanin0());
      case 0:
      case 3:
      case 4: _dfsList.push_back(g);
      default: break;
      }
   }
}

void
CirMgr::doDfs2(CirGate* g, vector<CirGate*> *dfsl )
{
   if( g->_color2==false ) {
      g->setColor2(true);
      switch(g->getType()) {
      case 2: doDfs2(g->getFanin0(),dfsl);
              doDfs2(g->getFanin1(),dfsl);
              break;
      case 1: doDfs2(g->getFanin0(),dfsl);
      default: break;
      }
      dfsl->push_back(g);
   }
}

void
CirMgr::printFECPairs() const
{
//unsigned count=0;
   for( unsigned i=0 ; i<_fecGrps.size() ; ++i ) {
      if( _fecGrps[i]->order()==4294967295 ) break;
//++count;
      cout<<"["<<i<<"]";
      for( unsigned j=0 ; j<_fecGrps[i]->size() ; ++j ) {
         cout<<" ";
         if( _fecGrps[i]->num(j)->second ) cout<<"!";
         cout<<_fecGrps[i]->num(j)->first->getGateId();
      }
      cout<<endl;
   }
//cout<<"There are "<<count<<" FEC groups."<<endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g)
{
   vector<CirGate*> dfsl;
   doDfs2(g,&dfsl);
   unsigned m=0,i=0,a=0;
   for( unsigned j=0 ; j<dfsl.size() ; ++j ) {
      switch( dfsl[j]->_type ) {
      case 0: ++i; break;
      case 2: ++a;
      default: break;
      }
      if( dfsl[j]->_gateId>m ) m=dfsl[j]->_gateId;
      dfsl[j]->_color2=false;
   }
   outfile<<"aag "<<m<<" "<<i<<" 0 1 "<<a<<endl;
   for( unsigned j=0 ; j<dfsl.size() ; ++j )
      if(dfsl[j]->_type==0) outfile<<dfsl[j]->_gateId*2<<endl;
   outfile<<g->_gateId*2<<endl;
   for( unsigned j=0 ; j<dfsl.size() ; ++j )
      if(dfsl[j]->_type==2) outfile<<dfsl[j]->_gateId*2<<" "<<dfsl[j]->getFaninId1()<<" "<<dfsl[j]->getFaninId2()<<endl;
   outfile<<"o0 "<<g->_gateId<<endl;
   outfile<<"c"<<endl;
   outfile<<"Write gate ("<<g->_gateId<<") by Tsai-Jong Chaing";
}

