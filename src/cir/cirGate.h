/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "cirDef.h"
#include "myHashMap.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
enum CirGateType { PI, PO, AIG, Const, Undef };
class CirGate
{
public:
   friend class CirMgr;
   CirGate(unsigned id, unsigned l, unsigned c, CirGateType t=Undef):
   _gateId(id), _line(l), _column(c), _type(t) { _color=_color2=false; _sim=0; }
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const {
      switch(_type) {
         case 0: return "PI";
         case 1: return "PO";
         case 2: return "AIG";
         case 3: return "Const";
         case 4: return "Undef";
         default: return 0;
      }
   }
   unsigned getLineNo() const { return _line; }
   unsigned getGateId() const { return _gateId; }
   bool getColor() const { return _color; }
   unsigned getType() const { return _type; }
   string getName() const { return _name; }
   unsigned getFanoutId() const { if(_fanoutList.empty()) return 0; else return _fanoutList[0]->getGateId(); }
   unsigned getFaninId1() const { if(_faninList.empty()) return 0; else if( _isInvert[0]==true )
      return _faninList[0]->getGateId()*2+1; else return _faninList[0]->getGateId()*2; }
   unsigned getFaninId2() const { if(_faninList.empty()) return 0; else if( _isInvert[1]==true )
      return _faninList[1]->getGateId()*2+1; else return _faninList[1]->getGateId()*2; }
   CirGate* getFanin0() const { return _faninList[0]; }
   CirGate* getFanin1() const { return _faninList[1]; }

   void addFanin(CirGate* g, bool isInvert) { _faninList.push_back(g); _isInvert.push_back(isInvert); }
   void addFanout(CirGate* g) { _fanoutList.push_back(g); }
   void deleteFanout(CirGate* g) { _fanoutList.erase(remove(_fanoutList.begin(),_fanoutList.end(),g),_fanoutList.end()); }
   void setFaninId1(CirGate* g, bool isInv) { _faninList[0]=g; _isInvert[0]=(isInv==true)?!_isInvert[0]:_isInvert[0]; }
   void setFaninId2(CirGate* g, bool isInv) { _faninList[1]=g; _isInvert[1]=(isInv==true)?!_isInvert[1]:_isInvert[1]; }

   void setType(CirGateType t) { _type=t; }
   void setLine(unsigned l) { _line=l; }
   void setCol(unsigned c) { _column=c; }
   void setName(string n) { _name=n; }
   void setColor(bool c) { _color=c; }
   void setColor2(bool c) { _color2=c; }

   virtual bool isAig() const { if(_type==2) return true; else return false; }

   // Printing functions
   virtual void printGate() const {}
   void reportGate() const;
   void reportFanin(int level);
   void reportFanout(int level);
   bool printFanin(bool& flag, unsigned& count);
   void outputFanin(bool& flag, ostream& outfile, unsigned& count, bool& toPrint);
   void trivialOpt(bool& flag, vector<unsigned>* eraseId, CirGate* const0);
   unsigned merge(CirGate *g2);
   unsigned mergeInv(CirGate *g2);
   //void strashFanin(bool& flag, HashMap<HashType,CirGate*>* hashMap);
   unsigned _sim;

private:
   unsigned _gateId;
   unsigned _line;
   unsigned _column;
   CirGateType _type;
   bool _color;
   bool _color2;
   Var _var;
   string _name;
   vector<CirGate *> _faninList;
   vector<CirGate *> _fanoutList;
   vector<bool> _isInvert;
   void trackFanin(int& level, int depth, bool& flag, CirGate *g, bool isInvert);
   void trackFanout(int& level, int depth, bool& flag, CirGate *g, bool isInvert);
};


typedef pair<CirGate*,bool> memberNode;

class fecGrp
{
public:
   friend class CirMgr;
   friend class CirGate;
   fecGrp(CirGate* g, bool b) { memberNode* m=new memberNode(make_pair(g,b)); _member.push_back(m); }
   fecGrp(memberNode* m) { _member.push_back(m); }
   ~fecGrp() { for( unsigned i=0 ; i<_member.size() ; ++i ) delete _member[i]; }
   void addMember(CirGate* g, bool b) { memberNode* m=new memberNode(make_pair(g,b)); _member.push_back(m); }
   //void addMember(memberNode* m) { _member.push_back(m); }
   memberNode* num(unsigned i) { return _member[i]; }
   unsigned size() { return _member.size(); }
   unsigned order() {
      if( _member[0]->second ) return 4294967295; else return _member[0]->first->getGateId(); }
private:
   vector<memberNode*> _member;
};

#endif // CIR_GATE_H
