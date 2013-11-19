/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Guido Tack <guido.tack@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <minizinc/optimize.hh>
#include <minizinc/hash.hh>
#include <minizinc/astiterator.hh>
#include <minizinc/prettyprinter.hh>
#include <minizinc/flatten.hh>

#include <unordered_set>

namespace MiniZinc {

  class VarOccurrences {
  public:
    typedef std::unordered_set<Item*> Items;
    ExpressionMap<Items> _m;
    
    void add(VarDecl* v, Item* i) {
      ExpressionMap<Items>::iterator vi = _m.find(v);
      if (vi==_m.end()) {
        Items items; items.insert(i);
        _m.insert(v, items);
      } else {
        vi->second.insert(i);
      }
    }
    
    /// Remove \a i from map and return new number of occurrences
    int remove(VarDecl* v, Item* i) {
      ExpressionMap<Items>::iterator vi = _m.find(v);
      assert(vi!=_m.end());
      vi->second.erase(i);
      return vi->second.size();
    }
    
    /// Return number of occurrences of \a v
    int occurrences(VarDecl* v) {
      ExpressionMap<Items>::iterator vi = _m.find(v);
      return (vi==_m.end() ? 0 : vi->second.size());
    }
    
  };
  
  class CollectOccurrencesE : public EVisitor {
  public:
    VarOccurrences& vo;
    Item* ci;
    CollectOccurrencesE(VarOccurrences& vo0, Item* ci0)
      : vo(vo0), ci(ci0) {}
    void vId(const Id& id) {
      if(id._decl)
        vo.add(id._decl,ci);
    }
    
  };

  class CollectOccurrencesI : public ItemVisitor {
  public:
    VarOccurrences& vo;
    CollectOccurrencesI(VarOccurrences& vo0) : vo(vo0) {}
    void vVarDeclI(VarDeclI* v) {
      CollectOccurrencesE ce(vo,v);
      topDown(ce,v->_e);
    }
    void vConstraintI(ConstraintI* ci) {
      CollectOccurrencesE ce(vo,ci);
      topDown(ce,ci->_e);
    }
    void vSolveI(SolveI* si) {
      CollectOccurrencesE ce(vo,si);
      topDown(ce,si->_e);
      topDown(ce,si->_ann);
    }
  };

  class AnnotateVardecl : public ItemVisitor {
  public:
    VarOccurrences& vo;
    AnnotateVardecl(VarOccurrences& vo0) : vo(vo0) {}
    void vVarDeclI(VarDeclI* v) {
      GCLock _gcl;
      std::vector<Expression*> args(1);
      args[0] = IntLit::a(Location(),vo.occurrences(v->_e));
      Call* c = Call::a(Location(),"occ",args);
      v->_e->annotate(Annotation::a(Location(),c));
    }
  };

  class CollectDecls : public EVisitor {
  public:
    VarOccurrences& vo;
    std::vector<VarDecl*>& vd;
    VarDeclI* vdi;
    CollectDecls(VarOccurrences& vo0,
                 std::vector<VarDecl*>& vd0,
                 VarDeclI* vdi0)
      : vo(vo0), vd(vd0), vdi(vdi0) {}
    void vId(Id& id) {
      if (id._decl && vo.remove(id._decl,vdi) == 0)
        vd.push_back(id._decl);
    }
  };

  void removeUnused(Model& m, VarOccurrences& vo) {
    ExpressionMap<int> idx;
    for (unsigned int i=0; i<m.size(); i++) {
      if (VarDeclI* vdi = m[i]->dyn_cast<VarDeclI>()) {
        idx.insert(vdi->_e,i);
      }
    }
    std::vector<VarDecl*> vd;
    int msize = m.size();
    for (unsigned int i=0; i<msize; i++) {
      VarDeclI* vdi = m[i]->dyn_cast<VarDeclI>();
      if (vdi!=NULL && vo.occurrences(vdi->_e)==0 ) {
        if (vdi->_e->_e && vdi->_e->_ti->_domain) {
          if (vdi->_e->_type.isvar() && vdi->_e->_type.isbool() &&
              Expression::equal(vdi->_e->_ti->_domain,constants().lt)) {
            GCLock lock;
            ConstraintI* ci = ConstraintI::a(vdi->_loc,vdi->_e->_e);
            if (vdi->_e->introduced()) {
              m[i] = ci;
            } else {
              vdi->_e->_e = NULL;
              m._items.push_back(ci);
            }
          } else if (vdi->_e->_ti->computedDomain()) {
            CollectDecls cd(vo,vd,vdi);
            topDown(cd,vdi->_e->_e);
            vdi->remove();
          }
        } else {
          CollectDecls cd(vo,vd,vdi);
          topDown(cd,vdi->_e->_e);
          vdi->remove();
        }
      }
    }
    while (!vd.empty()) {
      VarDecl* cur = vd.back(); vd.pop_back();
      ExpressionMap<int>::iterator cur_idx = idx.find(cur);
      if (cur_idx != idx.end() && !m[cur_idx->second]->removed()) {
        CollectDecls cd(vo,vd,m[cur_idx->second]->cast<VarDeclI>());
        topDown(cd,cur->_e);
        m[cur_idx->second]->remove();
      }
    }
    m.compact();
  }

  void optimize(Model* m) {
    VarOccurrences vo;
    CollectOccurrencesI co(vo);
    iterItems(co,m);
    // AnnotateVardecl avd(vo);
    // iterItems<AnnotateVardecl>(avd,m);
    removeUnused(*m,vo);
  }

}