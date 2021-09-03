
/*! @file
 * @brief Layout support routines
 *
 * The layout routines provide various layouts. Most of this
 * is architecture dependent, so lives in the ${arch}_specific.cc codes.
 * The routines here are some auxilliary routines.
 */

#include "qdp.h"
#include "qdp_util.h"

namespace QDP {

namespace
{
  template<class T>
  inline
  StandardOutputStream& operator<<(StandardOutputStream& s, const multi1d<T>& d)
  {
    for(int i=0; i < d.size(); ++i)
      s << d[i] << " ";
    return s;
  }
}

namespace Layout
{
  //! Returns the logical node coordinates for the corresponding lattice coordinate
  multi1d<int> nodeCoord(const multi1d<int>& coord) 
  {
    multi1d<int> logical(Nd);

    for(int i=0; i < Nd; ++i)
      logical[i] = coord[i] / subgridLattSize()[i];

    return logical;
  }

  extern "C" { 

    /* Export this to "C" */
    void QDPXX_getSiteCoords(int coord[], int node, int linear) QDP_CONST {
      multi1d<int> wrapped_coords = siteCoords(node,linear);
      for(int i=0; i < Nd; i++) { 
	coord[i] = wrapped_coords[i];
      }
    }
    
    int QDPXX_getLinearSiteIndex(const int coord[]) {
      multi1d<int> wrapped_coords(Nd);
      for(int i=0; i < Nd; i++) { 
	wrapped_coords[i]=coord[i];
      }
      return linearSiteIndex(wrapped_coords);
    }

    int QDPXX_nodeNumber(const int coord[]) {
      multi1d<int> wrapped_coords(Nd);
      for(int i=0; i < Nd; i++) { 
	wrapped_coords[i]=coord[i];
      }
      return nodeNumber(wrapped_coords);
    }
  }
}


#if QDP_USE_CB3D_LAYOUT == 1

  multi1d<int> crtesn(int ipos, const multi1d<int>& latt_size)
  {
    multi1d<int> coord(latt_size.size());
    int Ndim=latt_size.size() - 1; // Last elem latt size
    
    /* Calculate the Cartesian coordinates of the VALUE of IPOS where the 
     * value is defined by
     *
     *     for i = 0 to NDIM-1  {
     *        X_i  <- mod( IPOS, L(i) )
     *        IPOS <- int( IPOS / L(i) )
     *     }
     *
     * NOTE: here the coord(i) and IPOS have their origin at 0. 
     */
    for(int i = Ndim; i < Ndim+latt_size.size(); ++i)
      {
	int ix=i%latt_size.size();

	coord[ix] = ipos % latt_size[ix];
	ipos = ipos / latt_size[ix];
      }

  return coord;
}
  
//! Calculates the lexicographic site index from the coordinate of a site
/*! 
 * Nothing specific about the actual lattice size, can be used for 
 * any kind of latt size 
 */
int local_site(const multi1d<int>& coord, const multi1d<int>& latt_size)
{
  int order = 0;

  // In the 4D Case: t+Lt(x + Lx(y + Ly*z)
  // essentially  starting from i = dim[Nd-2]
  //  order =  latt_size[i-1]*(coord[i])
  //   and need to wrap i-1 around to Nd-1 when it gets below 0
  for(int mmu=latt_size.size()-2; mmu >= 0; --mmu) {
    int wrapmu = (mmu-1) % latt_size.size();
    if ( wrapmu < 0 ) wrapmu += latt_size.size();
    order = latt_size[wrapmu]*(coord[mmu] + order);
  }

  order += coord[ latt_size.size()-1 ];

  return order;
}


#else
  // Usual lattice decomposition -- x fastest, t slowest
  //! Decompose a lexicographic site into coordinates
multi1d<int> crtesn(int ipos, const multi1d<int>& latt_size)
{
  multi1d<int> coord(latt_size.size());

  /* Calculate the Cartesian coordinates of the VALUE of IPOS where the 
   * value is defined by
   *
   *     for i = 0 to NDIM-1  {
   *        X_i  <- mod( IPOS, L(i) )
   *        IPOS <- int( IPOS / L(i) )
   *     }
   *
   * NOTE: here the coord(i) and IPOS have their origin at 0. 
   */
  for(int i=0; i < latt_size.size(); ++i)
  {
    coord[i] = ipos % latt_size[i];
    ipos = ipos / latt_size[i];
  }

  return coord;
}
  
//! Calculates the lexicographic site index from the coordinate of a site
/*! 
 * Nothing specific about the actual lattice size, can be used for 
 * any kind of latt size 
 */
int local_site(const multi1d<int>& coord, const multi1d<int>& latt_size)
{
  int order = 0;

  for(int mmu=latt_size.size()-1; mmu >= 1; --mmu)
    order = latt_size[mmu-1]*(coord[mmu] + order);

  order += coord[0];

  return order;
}

#endif



#if QDP_USE_VNODE_LAYOUT == 1
namespace Layout
{
  multi1d<int> m_vn_geom;
  multi1d<int> m_vn_subgrid;

  void printVirtualNodeInfo()
  {
    if (m_vn_geom.size() != Nd)
      {
	QDPIO::cout << "Virtual node size not set!" << std::endl;
	QDP_abort(1);
      }

    QDPIO::cout << "Virtual node setup:" << std::endl;
    QDPIO::cout << "  virtual node geometry     = " << Layout::virtualNodeGeom() << std::endl;
    QDPIO::cout << "  virtual node subgrid      = " << Layout::virtualNodeSubgridLattSize() << std::endl;
    QDPIO::cout << "  sites within virtual node = " << Layout::virtualNodeSites() << std::endl;
    QDPIO::cout << "  number of virtual nodes   = " << Layout::virtualNodeNumber() << std::endl;
  
#if 0  
    QDPIO::cout << "using default vnode geometry of ";
    for(int i=0; i < Nd; i++) 
      QDPIO::cout << n[i] << " ";
    QDPIO::cout << std::endl;
#endif
  }
  
  void setVirtualNodeGeom(const multi1d<int>& g)
  {
    if (g.size() != Nd)
      {
	std::cerr << "virtual node geometry not " << Nd << " dimensional." << std::endl;
	QDP_abort(1);
      }
    m_vn_geom = g;
  }

  void initVirtualNode()
  {
    m_vn_subgrid.resize(Nd);
    for( int i = 0 ; i < m_vn_geom.size() ; ++i )
      m_vn_subgrid[i] = Layout::subgridLattSize()[i] / m_vn_geom[i];
  }
  
  multi1d<int> virtualNodeGeom()
  {
    return m_vn_geom;
  }
  
  multi1d<int> virtualNodeSubgridLattSize()
  {
    return m_vn_subgrid;
  }

  int virtualNodeSites()
  {
    int tmp = virtualNodeSubgridLattSize()[0];
    for( int i = 1 ; i < virtualNodeSubgridLattSize().size() ; ++i )
      tmp *= virtualNodeSubgridLattSize()[i];
    return tmp;
  }

  int virtualNodeNumber()
  {
    int tmp = virtualNodeGeom()[0];
    for( int i = 1 ; i < virtualNodeGeom().size() ; ++i )
      tmp *= virtualNodeGeom()[i];
    return tmp;
  }
} // Layout
#endif  // VNODE layout
  


} // namespace QDP;
