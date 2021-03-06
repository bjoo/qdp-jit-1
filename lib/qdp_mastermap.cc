// -*- C++ -*-

#include "qdp.h"
#include "qdp_util.h"

namespace QDP {


  MasterMap& MasterMap::Instance()
  {
    static MasterMap singleton;
    return singleton;
  }


  void MasterMap::remove_neg(multi1d<int>& out, const multi1d<int>& orig) const {
    multi1d<int> c(Layout::sitesOnNode());
    int num=0;
    for(int i=0 ; i<Layout::sitesOnNode() ; ++i) 
      if (orig[i] >= 0)
	c[num++]=i;
    out.resize( num );
    for(int i=0; i < num; ++i)
      out[i] = c[i];
  }



  void MasterMap::remove_neg_in_subset(multi1d<int>& out, const multi1d<int>& orig, int s_no ) const 
  {
    const Subset& subset = MasterSet::Instance().getSubset(s_no);

    multi1d<int> c(Layout::sitesOnNode());

    int num=0;
    for(int i=0 ; i<Layout::sitesOnNode() ; ++i) 
      if (orig[i] >= 0 && subset.isElement(i))
	c[num++]=i;

    out.resize( num );

    for(int i=0; i < num; ++i)
      out[i] = c[i];
  }



  void MasterMap::uniquify_list_inplace(multi1d<int>& out , const multi1d<int>& ll) const
  {
    multi1d<int> d(ll.size());

    // Enter the first element as unique to prime the search
    int ipos = 0;
    int num = 0;
    int prev_node;
  
    d[num++] = prev_node = ll[ipos++];

    // Find the unique source nodes
    while (ipos < ll.size())
      {
	int this_node = ll[ipos++];

	if (this_node != prev_node)
	  {
	    // Has this node occured before?
	    bool found = false;
	    for(int i=0; i < num; ++i)
	      if (d[i] == this_node)
		{
		  found = true;
		  break;
		}

	    // If this is the first time this value has occurred, enter it
	    if (! found)
	      d[num++] = this_node;
	  }

	prev_node = this_node;
      }

    // Copy into a compact size array
    out.resize(num);
    for(int i=0; i < num; ++i) {
      out[i] = d[i];
    }

  }


  void MasterMap::registrate_work(const Map& map, const Subset& subset) {

    int id = map.getId();
    int s_no = subset.getId();

    if (powerSet[ s_no].size() > id) {
      return;
    }
    
    {
      if (powerSet[ s_no].size() < id)
	{
	  int tmp = id;
	  int log2id = 0;
	  while (tmp >>= 1) ++log2id;
	  assert(log2id > 0);
	  log2id--;
	  assert( vecPMap.size() > log2id );
	  registrate_work( *vecPMap.at(log2id) , subset );
	}

      assert( powerSet[ s_no].size() == id );

      powerSet[s_no].resize( id << 1 );
      powerSetC[s_no].resize( id << 1 );
      idInner[s_no].resize( id << 1 );
      idFace[s_no].resize( id << 1 );

      for (int i = 0 ; i < id ; ++i ) {
	
	multi1d<int> ct(Layout::sitesOnNode()); // complement, inner region
	multi1d<int> pt(Layout::sitesOnNode()); // positive, union of receive sites
	for(int q=0 ; q<Layout::sitesOnNode() ; ++q) {
	  ct[q]=q;
	  pt[q]=-1;
	}

	for (int q = 0 ; q < powerSet[s_no][i]->size() ; ++q ) {    // !!
	  ct[ (*powerSet[s_no][i])[q] ] = -1;
	  pt[ (*powerSet[s_no][i])[q] ] = (*powerSet[s_no][i])[q];
	}

	map.getRoffsetsId( subset ); // make sure the lazy part was computed!
	for (int q = 0; q < map.roffset( subset ).size() ; ++q ) {
	  ct[ map.roffset( subset )[q] ] = -1;
	  pt[ map.roffset( subset )[q] ] = map.roffset( subset )[q];
	}

	powerSet[s_no][i|id] = new multi1d<int>;
	powerSetC[s_no][i|id]= new multi1d<int>;

	remove_neg_in_subset( *powerSetC[s_no][i|id] , ct , s_no );
	remove_neg_in_subset( *powerSet[s_no][i|id] , pt , s_no );

	assert( idFace.size() > s_no );
	assert( idFace[s_no].size() > (i|id) );

	assert( idInner.size() > s_no );
	assert( idInner[s_no].size() > (i|id) );

	idFace[s_no][i|id] = QDP_get_global_cache().registrateOwnHostMem( powerSet[s_no][i|id]->size() * sizeof(int) , 
									  powerSet[s_no][i|id]->slice() , NULL );

	idInner[s_no][i|id] = QDP_get_global_cache().registrateOwnHostMem( powerSetC[s_no][i|id]->size() * sizeof(int) , 
									   powerSetC[s_no][i|id]->slice() , NULL );

      }
    }

  }


  int MasterMap::registrate_justid(const Map& map) {
    int id = 1 << vecPMap.size();
    vecPMap.push_back(&map);
    return id;
  }

  
  int MasterMap::getIdInner(const Subset& s,int bitmask) const {
    assert( s.getId() >= 0 && (unsigned)s.getId() < idInner.size() && "subset Id out of range");
    assert( bitmask > 0 && (unsigned)bitmask < idInner[s.getId()].size() && "bitmask out of range");
    return idInner[s.getId()][bitmask]; 
  }
  int MasterMap::getIdFace(const Subset& s,int bitmask) const {
    assert( s.getId() >= 0 && (unsigned)s.getId() < idFace.size() && "subset Id out of range");
    assert( bitmask > 0 && (unsigned)bitmask < idFace[s.getId()].size() && "bitmask out of range");
    return idFace[s.getId()][bitmask];
  }
  int MasterMap::getCountInner(const Subset& s,int bitmask) const {
    assert( s.getId() >= 0 && (unsigned)s.getId() < powerSet.size() && "subset Id out of range");
    assert( bitmask > 0 && (unsigned)bitmask < powerSetC[s.getId()].size() && "bitmask out of range");
    return powerSetC[s.getId()][bitmask]->size(); 
  }
  int MasterMap::getCountFace(const Subset& s,int bitmask) const {
    assert( s.getId() >= 0 && (unsigned)s.getId() < powerSet.size() && "subset Id out of range");
    assert( bitmask > 0 && (unsigned)bitmask < powerSet[s.getId()].size() && "bitmask out of range");
    return powerSet[s.getId()][bitmask]->size(); 
  }


} // namespace QDP


