#pragma once

#include "2d.h"

namespace dt_ocv
{
// opencv
// inconsistant adherance to T=2N-K-2

/*
//! @addtogroup imgproc_subdiv2d
//! @{

class CV_EXPORTS_W Subdiv2D
{
public:
    /** Subdiv2D point location cases */
 /*   enum { PTLOC_ERROR        = -2, //!< Point location error
           PTLOC_OUTSIDE_RECT = -1, //!< Point outside the subdivision bounding rect
           PTLOC_INSIDE       = 0, //!< Point inside some facet
           PTLOC_VERTEX       = 1, //!< Point coincides with one of the subdivision vertices
           PTLOC_ON_EDGE      = 2  //!< Point on some edge
         };*/

    /** Subdiv2D edge type navigation (see: getEdge()) */
/*    enum { NEXT_AROUND_ORG   = 0x00,
           NEXT_AROUND_DST   = 0x22,
           PREV_AROUND_ORG   = 0x11,
           PREV_AROUND_DST   = 0x33,
           NEXT_AROUND_LEFT  = 0x13,
           NEXT_AROUND_RIGHT = 0x31,
           PREV_AROUND_LEFT  = 0x20,
           PREV_AROUND_RIGHT = 0x02
         };
         */
    /** creates an empty Subdiv2D object.
    To create a new empty Delaunay subdivision you need to use the #initDelaunay function.
     */
  //  CV_WRAP Subdiv2D();

    /** @overload

    @param rect Rectangle that includes all of the 2D points that are to be added to the subdivision.

    The function creates an empty Delaunay subdivision where 2D points can be added using the function
    insert() . All of the points to be added must be within the specified rectangle, otherwise a runtime
    error is raised.
     */
 //   CV_WRAP Subdiv2D(Rect rect);

    /** @brief Creates a new empty Delaunay subdivision

    @param rect Rectangle that includes all of the 2D points that are to be added to the subdivision.

     */
 //   CV_WRAP void initDelaunay(Rect rect);

    /** @brief Insert a single point into a Delaunay triangulation.

    @param pt Point to insert.

    The function inserts a single point into a subdivision and modifies the subdivision topology
    appropriately. If a point with the same coordinates exists already, no new point is added.
    @returns the ID of the point.

    @note If the point is outside of the triangulation specified rect a runtime error is raised.
     */
 //   CV_WRAP int insert(Point2f pt);

    /** @brief Insert multiple points into a Delaunay triangulation.

    @param ptvec Points to insert.

    The function inserts a vector of points into a subdivision and modifies the subdivision topology
    appropriately.
     */
 //   CV_WRAP void insert(const std::vector<Point2f>& ptvec);

    /** @brief Returns the location of a point within a Delaunay triangulation.

    @param pt Point to locate.
    @param edge Output edge that the point belongs to or is located to the right of it.
    @param vertex Optional output vertex the input point coincides with.

    The function locates the input point within the subdivision and gives one of the triangle edges
    or vertices.

    @returns an integer which specify one of the following five cases for point location:
    -  The point falls into some facet. The function returns #PTLOC_INSIDE and edge will contain one of
       edges of the facet.
    -  The point falls onto the edge. The function returns #PTLOC_ON_EDGE and edge will contain this edge.
    -  The point coincides with one of the subdivision vertices. The function returns #PTLOC_VERTEX and
       vertex will contain a pointer to the vertex.
    -  The point is outside the subdivision reference rectangle. The function returns #PTLOC_OUTSIDE_RECT
       and no pointers are filled.
    -  One of input arguments is invalid. A runtime error is raised or, if silent or "parent" error
       processing mode is selected, #PTLOC_ERROR is returned.
     */
  //  CV_WRAP int locate(Point2f pt, CV_OUT int& edge, CV_OUT int& vertex);

    /** @brief Finds the subdivision vertex closest to the given point.

    @param pt Input point.
    @param nearestPt Output subdivision vertex point.

    The function is another function that locates the input point within the subdivision. It finds the
    subdivision vertex that is the closest to the input point. It is not necessarily one of vertices
    of the facet containing the input point, though the facet (located using locate() ) is used as a
    starting point.

    @returns vertex ID.
     */
  //  CV_WRAP int findNearest(Point2f pt, CV_OUT Point2f* nearestPt = 0);

    /** @brief Returns a list of all edges.

    @param edgeList Output vector.

    The function gives each edge as a 4 numbers vector, where each two are one of the edge
    vertices. i.e. org_x = v[0], org_y = v[1], dst_x = v[2], dst_y = v[3].
     */
  //  CV_WRAP void getEdgeList(CV_OUT std::vector<Vec4f>& edgeList) const;

    /** @brief Returns a list of the leading edge ID connected to each triangle.

    @param leadingEdgeList Output vector.

    The function gives one edge ID for each triangle.
     */
  //  CV_WRAP void getLeadingEdgeList(CV_OUT std::vector<int>& leadingEdgeList) const;

    /** @brief Returns a list of all triangles.

    @param triangleList Output vector.

    The function gives each triangle as a 6 numbers vector, where each two are one of the triangle
    vertices. i.e. p1_x = v[0], p1_y = v[1], p2_x = v[2], p2_y = v[3], p3_x = v[4], p3_y = v[5].
     */
  //  CV_WRAP void getTriangleList(CV_OUT std::vector<Vec6f>& triangleList) const;

    /** @brief Returns a list of all Voronoi facets.

    @param idx Vector of vertices IDs to consider. For all vertices you can pass empty vector.
    @param facetList Output vector of the Voronoi facets.
    @param facetCenters Output vector of the Voronoi facets center points.

     */
  //  CV_WRAP void getVoronoiFacetList(const std::vector<int>& idx, CV_OUT std::vector<std::vector<Point2f> >& facetList,
                     //                CV_OUT std::vector<Point2f>& facetCenters);

    /** @brief Returns vertex location from vertex ID.

    @param vertex vertex ID.
    @param firstEdge Optional. The first edge ID which is connected to the vertex.
    @returns vertex (x,y)

     */
   // CV_WRAP Point2f getVertex(int vertex, CV_OUT int* firstEdge = 0) const;

    /** @brief Returns one of the edges related to the given edge.

    @param edge Subdivision edge ID.
    @param nextEdgeType Parameter specifying which of the related edges to return.
    The following values are possible:
    -   NEXT_AROUND_ORG next around the edge origin ( eOnext on the picture below if e is the input edge)
    -   NEXT_AROUND_DST next around the edge vertex ( eDnext )
    -   PREV_AROUND_ORG previous around the edge origin (reversed eRnext )
    -   PREV_AROUND_DST previous around the edge destination (reversed eLnext )
    -   NEXT_AROUND_LEFT next around the left facet ( eLnext )
    -   NEXT_AROUND_RIGHT next around the right facet ( eRnext )
    -   PREV_AROUND_LEFT previous around the left facet (reversed eOnext )
    -   PREV_AROUND_RIGHT previous around the right facet (reversed eDnext )

    ![sample output](pics/quadedge.png)

    @returns edge ID related to the input edge.
     */
   // CV_WRAP int getEdge( int edge, int nextEdgeType ) const;

    /** @brief Returns next edge around the edge origin.

    @param edge Subdivision edge ID.

    @returns an integer which is next edge ID around the edge origin: eOnext on the
    picture above if e is the input edge).
     */
  //  CV_WRAP int nextEdge(int edge) const;

    /** @brief Returns another edge of the same quad-edge.

    @param edge Subdivision edge ID.
    @param rotate Parameter specifying which of the edges of the same quad-edge as the input
    one to return. The following values are possible:
    -   0 - the input edge ( e on the picture below if e is the input edge)
    -   1 - the rotated edge ( eRot )
    -   2 - the reversed edge (reversed e (in green))
    -   3 - the reversed rotated edge (reversed eRot (in green))

    @returns one of the edges ID of the same quad-edge as the input edge.
     */
    //CV_WRAP int rotateEdge(int edge, int rotate) const;
   // CV_WRAP int symEdge(int edge) const;

    /** @brief Returns the edge origin.

    @param edge Subdivision edge ID.
    @param orgpt Output vertex location.

    @returns vertex ID.
     */
  //  CV_WRAP int edgeOrg(int edge, CV_OUT Point2f* orgpt = 0) const;

    /** @brief Returns the edge destination.

    @param edge Subdivision edge ID.
    @param dstpt Output vertex location.

    @returns vertex ID.
     */
/*    CV_WRAP int edgeDst(int edge, CV_OUT Point2f* dstpt = 0) const;

protected:
    int newEdge();
    void deleteEdge(int edge);
    int newPoint(Point2f pt, bool isvirtual, int firstEdge = 0);
    void deletePoint(int vtx);
    void setEdgePoints( int edge, int orgPt, int dstPt );
    void splice( int edgeA, int edgeB );
    int connectEdges( int edgeA, int edgeB );
    void swapEdges( int edge );
    int isRightOf(Point2f pt, int edge) const;
    void calcVoronoi();
    void clearVoronoi();
    void checkSubdiv() const;

    struct CV_EXPORTS Vertex
    {
        Vertex();
        Vertex(Point2f pt, bool isvirtual, int firstEdge=0);
        bool isvirtual() const;
        bool isfree() const;

        int firstEdge;
        int type;
        Point2f pt;
    };

    struct CV_EXPORTS QuadEdge
    {
        QuadEdge();
        QuadEdge(int edgeidx);
        bool isfree() const;

        int next[4];
        int pt[4];
    };

    //! All of the vertices
    std::vector<Vertex> vtx;
    //! All of the edges
    std::vector<QuadEdge> qedges;
    int freeQEdge;
    int freePoint;
    bool validGeometry;

    int recentEdge;
    //! Top left corner of the bounding rect
    Point2f topLeft;
    //! Bottom right corner of the bounding rect
    Point2f bottomRight;
};
*/

template <typename T=MOR_FLTTYPE> class subdiv
{
public:
    enum ORIEN {ORIEN_CW,ORIEN_CCW};
    enum { PTLOC_ERROR        = -2, //!< Point location error
           PTLOC_OUTSIDE_RECT = -1, //!< Point outside the subdivision bounding rect
           PTLOC_INSIDE       = 0, //!< Point inside some facet
           PTLOC_VERTEX       = 1, //!< Point coincides with one of the subdivision vertices
           PTLOC_ON_EDGE      = 2  //!< Point on some edge
         };
    enum { NEXT_AROUND_ORG   = 0x00,
           NEXT_AROUND_DST   = 0x22,
           PREV_AROUND_ORG   = 0x11,
           PREV_AROUND_DST   = 0x33,
           NEXT_AROUND_LEFT  = 0x13,
           NEXT_AROUND_RIGHT = 0x31,
           PREV_AROUND_LEFT  = 0x20,
           PREV_AROUND_RIGHT = 0x02
         };

	subdiv()
    {
        validGeometry = false;
        freeQEdge = 0;
        freePoint = 0;
        recentEdge = 0;
    }
	~subdiv(){}

    int insert(const af2d::point<T>& pt)
    {
        int curr_point = 0, curr_edge = 0, deleted_edge = 0;
        int location = locate( pt, curr_edge, curr_point );

        if( location == PTLOC_ERROR )
            return -1;//CV_Error( cv::Error::StsBadSize, "" );

        if( location == PTLOC_OUTSIDE_RECT )
            return -1;//CV_Error( cv::Error::StsOutOfRange, "" );

        if( location == PTLOC_VERTEX )
            return curr_point;

        if( location == PTLOC_ON_EDGE )
        {
            deleted_edge = curr_edge;
            recentEdge = curr_edge = getEdge( curr_edge, PREV_AROUND_ORG );
            deleteEdge(deleted_edge);
        }
        else if( location == PTLOC_INSIDE )
            ;
        else
            return -1;//CV_Error_(cv::Error::StsError, ("Subdiv2D::locate returned invalid location = %d", location) );

        //CV_Assert( curr_edge != 0 );
        validGeometry = false;

        curr_point = newPoint(pt, false);
        int base_edge = newEdge();
        int first_point = edgeOrg(curr_edge);
        setEdgePoints(base_edge, first_point, curr_point);
        splice(base_edge, curr_edge);

        do
        {
            base_edge = connectEdges( curr_edge, symEdge(base_edge) );
            curr_edge = getEdge(base_edge, PREV_AROUND_ORG);
        }
        while( edgeDst(curr_edge) != first_point );

        curr_edge = getEdge( base_edge, PREV_AROUND_ORG );

        int i, max_edges = (int)(qedges.size()*4);

        for( i = 0; i < max_edges; i++ )
        {
            int temp_dst = 0, curr_org = 0, curr_dst = 0;
            int temp_edge = getEdge( curr_edge, PREV_AROUND_ORG );

            temp_dst = edgeDst( temp_edge );
            curr_org = edgeOrg( curr_edge );
            curr_dst = edgeDst( curr_edge );

            if( isRightOf( vtx[temp_dst].pt, curr_edge ) > 0 &&
                isPtInCircle3( vtx[curr_org].pt, vtx[temp_dst].pt,
                vtx[curr_dst].pt, vtx[curr_point].pt ) < 0 )
            {
                swapEdges( curr_edge );
                curr_edge = getEdge( curr_edge, PREV_AROUND_ORG );
            }
            else if( curr_org == first_point )
                break;
            else
                curr_edge = getEdge( nextEdge( curr_edge ), PREV_AROUND_LEFT );
        }

        return curr_point;
    }
	struct tri
	{
        tri(){abcIdx[0]=-1;abcIdx[1]=-1;abcIdx[2]=-1;}
        tri(const T dAX,const T dAY,const T dBX,const T dBY,const T dCX,const T dCY):tri(){abc[0].getx()=dAX;abc[0].gety()=dAY;abc[1].getx()=dBX;abc[1].gety()=dBY;abc[2].getx()=dCX;abc[2].gety()=dCY;}

        // logical space
        af2d::point<T> abc[3];              // vertices
		int abcIdx[3];						// id comprised of 3 vertex indices
        af2d::rect<T> bbox;                 // bbox
	    af2d::point<T> ab;                  // cached edge
	    af2d::point<T> bc;                  // cached edge
	    af2d::point<T> ca;                  // cached edge
    };
    void getTriangleList(const ORIEN o,std::vector<tri>& triangleList,const bool bClip) const
    {
        const bool bCW=(o==ORIEN_CW);

        triangleList.clear();
        int i, total = (int)(qedges.size()*4);
        std::vector<bool> edgemask(total, false);
        af2d::rect<T> rect(topLeft,bottomRight);
        
        for( i = 4; i < total; i += 2 )
        {
            if( edgemask[i] )
                continue;
            af2d::point<T> a, b, c;
            int edge_a = i;
            const int v_a=edgeOrg(edge_a, &a);
            if ( bClip && !rect.isinside(a) )
                continue;
            int edge_b = getEdge(edge_a, NEXT_AROUND_LEFT);
            const int v_b=edgeOrg(edge_b, &b);
            if ( bClip && !rect.isinside(b) )
                continue;
            int edge_c = getEdge(edge_b, NEXT_AROUND_LEFT);
            const int v_c=edgeOrg(edge_c, &c);
            if ( bClip && !rect.isinside(c) )
                continue;
            edgemask[edge_a] = true;
            edgemask[edge_b] = true;
            edgemask[edge_c] = true;

            bool bFlip=false;
            {
                const T dArea=0.5*( a.getx()*(b.gety()-c.gety()) + b.getx()*(c.gety()-a.gety()) + c.getx()*(a.gety()-b.gety()) );
                if(dArea==0.0)
                {
                    // colinear - no nothing
                }
                else
                if(dArea<0.0)
                {
                    // cartesian space CW, dib space CCW
                    bFlip=bCW;
                }
                else
                {
                    // cartesian space CCW, dib space CW
                    bFlip=!bCW;
                }
            }

            if(bFlip)
                triangleList.push_back({c.getx(), c.gety(),b.getx(), b.gety(),a.getx(), a.gety()});
            else
                triangleList.push_back({a.getx(), a.gety(),b.getx(), b.gety(),c.getx(), c.gety()});
        }
    }
    void initDelaunay( const af2d::rect<>& rect )
    {
        T big_coord = 3.f * af::maxval<T>(rect.getwidth(), rect.getheight() );
        T rx = (T)rect.get(af2d::rect<>::tl).getx();
        T ry = (T)rect.get(af2d::rect<>::tl).gety();

        vtx.clear();
        qedges.clear();

        recentEdge = 0;
        validGeometry = false;

        topLeft = af2d::point<T>( rx, ry );
        bottomRight = af2d::point<T>( rx + rect.getwidth(), ry + rect.getheight() );

        af2d::point<T> ppA( rx + big_coord, ry );
        af2d::point<T> ppB( rx, ry + big_coord );
        af2d::point<T> ppC( rx - big_coord, ry - big_coord );

        vtx.push_back(Vertex());
        qedges.push_back(QuadEdge());

        freeQEdge = 0;
        freePoint = 0;

        int pA = newPoint(ppA, false);
        int pB = newPoint(ppB, false);
        int pC = newPoint(ppC, false);

        int edge_AB = newEdge();
        int edge_BC = newEdge();
        int edge_CA = newEdge();

        setEdgePoints( edge_AB, pA, pB );
        setEdgePoints( edge_BC, pB, pC );
        setEdgePoints( edge_CA, pC, pA );

        splice( edge_AB, symEdge( edge_CA ));
        splice( edge_BC, symEdge( edge_AB ));
        splice( edge_CA, symEdge( edge_BC ));

        recentEdge = edge_AB;
    }
protected:
    struct Vertex
    {
        Vertex(){firstEdge = 0;type = -1;}
        Vertex(const af2d::point<T>& _pt, bool _isvirtual, int _firstEdge=0){firstEdge = _firstEdge;type = (int)_isvirtual;pt = _pt;}
        bool isvirtual() const{return type > 0;}
        bool isfree() const{return type < 0;}

        int firstEdge;
        int type;
        af2d::point<T> pt;
    };
    struct QuadEdge
    {
        QuadEdge(){next[0] = next[1] = next[2] = next[3] = 0;pt[0] = pt[1] = pt[2] = pt[3] = 0;}
        QuadEdge(int edgeidx)
        {
            next[0] = edgeidx;
            next[1] = edgeidx+3;
            next[2] = edgeidx+2;
            next[3] = edgeidx+1;

            pt[0] = pt[1] = pt[2] = pt[3] = 0;
        }
        bool isfree() const{return next[0] <= 0;}

        int next[4];
        int pt[4];
    };
    std::vector<Vertex> vtx;
    std::vector<QuadEdge> qedges;
    int recentEdge;
    int freeQEdge;
    int freePoint;
    bool validGeometry;
    af2d::point<T> topLeft;
    af2d::point<T> bottomRight;

    int newPoint(const af2d::point<T>& pt, bool isvirtual, int firstEdge = 0)
    {
        if( freePoint == 0 )
        {
            vtx.push_back(Vertex());
            freePoint = (int)(vtx.size()-1);
        }
        int vidx = freePoint;
        freePoint = vtx[vidx].firstEdge;
        vtx[vidx] = Vertex(pt, isvirtual, firstEdge);

        return vidx;
    }
    int newEdge()
    {
        if( freeQEdge <= 0 )
        {
            qedges.emplace_back();
            freeQEdge = (int)(qedges.size()-1);
        }
        int edge = freeQEdge*4;
        freeQEdge = qedges[edge >> 2].next[1];
        qedges[edge >> 2] = QuadEdge(edge);
        return edge;
    }
    void setEdgePoints( int edge, int orgPt, int dstPt )
    {
        qedges[edge >> 2].pt[edge & 3] = orgPt;
        qedges[edge >> 2].pt[(edge + 2) & 3] = dstPt;
        vtx[orgPt].firstEdge = edge;
        vtx[dstPt].firstEdge = edge ^ 2;
    }
    int symEdge(int edge) const{return edge ^ 2;}
    void splice( int edgeA, int edgeB )
    {
        int& a_next = qedges[edgeA >> 2].next[edgeA & 3];
        int& b_next = qedges[edgeB >> 2].next[edgeB & 3];
        int a_rot = rotateEdge(a_next, 1);
        int b_rot = rotateEdge(b_next, 1);
        int& a_rot_next = qedges[a_rot >> 2].next[a_rot & 3];
        int& b_rot_next = qedges[b_rot >> 2].next[b_rot & 3];
        std::swap(a_next, b_next);
        std::swap(a_rot_next, b_rot_next);
    }
    int rotateEdge(int edge, int rotate) const{return (edge & ~3) + ((edge + rotate) & 3);}

    int locate(const af2d::point<T>& pt, int& _edge, int& _vertex)
    {
        int vertex = 0;

        int i, maxEdges = (int)(qedges.size() * 4);

        if( qedges.size() < (size_t)4 )
            return -1;//CV_Error( cv::Error::StsError, "Subdivision is empty" );

        if( pt.getx() < topLeft.getx() || pt.gety() < topLeft.gety() || pt.getx() >= bottomRight.getx() || pt.gety() >= bottomRight.gety() )
            return -1;//CV_Error( cv::Error::StsOutOfRange, "" );

        int edge = recentEdge;
        //CV_Assert(edge > 0);

        int location = PTLOC_ERROR;

        int right_of_curr = isRightOf(pt, edge);
        if( right_of_curr > 0 )
        {
            edge = symEdge(edge);
            right_of_curr = -right_of_curr;
        }

        for( i = 0; i < maxEdges; i++ )
        {
            int onext_edge = nextEdge( edge );
            int dprev_edge = getEdge( edge, PREV_AROUND_DST );

            int right_of_onext = isRightOf( pt, onext_edge );
            int right_of_dprev = isRightOf( pt, dprev_edge );

            if( right_of_dprev > 0 )
            {
                if( right_of_onext > 0 || (right_of_onext == 0 && right_of_curr == 0) )
                {
                    location = PTLOC_INSIDE;
                    break;
                }
                else
                {
                    right_of_curr = right_of_onext;
                    edge = onext_edge;
                }
            }
            else
            {
                if( right_of_onext > 0 )
                {
                    if( right_of_dprev == 0 && right_of_curr == 0 )
                    {
                        location = PTLOC_INSIDE;
                        break;
                    }
                    else
                    {
                        right_of_curr = right_of_dprev;
                        edge = dprev_edge;
                    }
                }
                else if( right_of_curr == 0 &&
                        isRightOf( vtx[edgeDst(onext_edge)].pt, edge ) >= 0 )
                {
                    edge = symEdge( edge );
                }
                else
                {
                    right_of_curr = right_of_onext;
                    edge = onext_edge;
                }
            }
        }

        recentEdge = edge;

        if( location == PTLOC_INSIDE )
        {
            af2d::point<T> org_pt, dst_pt;
            edgeOrg(edge, &org_pt);
            edgeDst(edge, &dst_pt);

            double t1 = fabs( pt.getx() - org_pt.getx() );
            t1 += fabs( pt.gety() - org_pt.gety() );
            double t2 = fabs( pt.getx() - dst_pt.getx() );
            t2 += fabs( pt.gety() - dst_pt.gety() );
            double t3 = fabs( org_pt.getx() - dst_pt.getx() );
            t3 += fabs( org_pt.gety() - dst_pt.gety() );

            if( t1 < FLT_EPSILON )
            {
                location = PTLOC_VERTEX;
                vertex = edgeOrg( edge );
                edge = 0;
            }
            else if( t2 < FLT_EPSILON )
            {
                location = PTLOC_VERTEX;
                vertex = edgeDst( edge );
                edge = 0;
            }
            else if( (t1 < t3 || t2 < t3) &&
                fabs( triangleArea( pt, org_pt, dst_pt )) < FLT_EPSILON )
            {
                location = PTLOC_ON_EDGE;
                vertex = 0;
            }
        }

        if( location == PTLOC_ERROR )
        {
            edge = 0;
            vertex = 0;
        }

        _edge = edge;
        _vertex = vertex;

        return location;
    }
    int getEdge( int edge, int nextEdgeType ) const
    {
        //CV_DbgAssert((size_t)(edge >> 2) < qedges.size());
        edge = qedges[edge >> 2].next[(edge + nextEdgeType) & 3];
        return (edge & ~3) + ((edge + (nextEdgeType >> 4)) & 3);
    }
    void deleteEdge(int edge)
    {
       // CV_DbgAssert((size_t)(edge >> 2) < (size_t)qedges.size());
        splice( edge, getEdge(edge, PREV_AROUND_ORG) );
        int sedge = symEdge(edge);
        splice(sedge, getEdge(sedge, PREV_AROUND_ORG) );

        edge >>= 2;
        qedges[edge].next[0] = 0;
        qedges[edge].next[1] = freeQEdge;
        freeQEdge = edge;
    }
    int edgeOrg(int edge, af2d::point<T>* orgpt = 0) const
    {
        // CV_DbgAssert((size_t)(edge >> 2) < qedges.size());
        int vidx = qedges[edge >> 2].pt[edge & 3];
        if( orgpt )
        {
            //CV_DbgAssert((size_t)vidx < vtx.size());
            *orgpt = vtx[vidx].pt;
        }
        return vidx;
    }
    int connectEdges( int edgeA, int edgeB )
    {
        int edge = newEdge();

        splice(edge, getEdge(edgeA, NEXT_AROUND_LEFT));
        splice(symEdge(edge), edgeB);

        setEdgePoints(edge, edgeDst(edgeA), edgeOrg(edgeB));
        return edge;
    }
    int edgeDst(int edge, af2d::point<T>* dstpt = 0) const
    {
       // CV_DbgAssert((size_t)(edge >> 2) < qedges.size());
        int vidx = qedges[edge >> 2].pt[(edge + 2) & 3];
        if( dstpt )
        {
            //CV_DbgAssert((size_t)vidx < vtx.size());
            *dstpt = vtx[vidx].pt;
        }
        return vidx;
    }
    int isRightOf(const af2d::point<T>& pt, int edge) const
    {
        af2d::point<T> org, dst;
        edgeOrg(edge, &org);
        edgeDst(edge, &dst);
        double cw_area = triangleArea( pt, dst, org );

        return (cw_area > 0) - (cw_area < 0);
    }
    inline int
    isPtInCircle3( const af2d::point<T>&  pt, const af2d::point<T>&  a, const af2d::point<T>&  b, const af2d::point<T>& c)
    {
        const double eps = FLT_EPSILON*0.125;
        double val = ((double)a.getx() * a.getx() + (double)a.gety() * a.gety()) * triangleArea( b, c, pt );
        val -= ((double)b.getx() * b.getx() + (double)b.gety() * b.gety()) * triangleArea( a, c, pt );
        val += ((double)c.getx() * c.getx() + (double)c.gety() * c.gety()) * triangleArea( a, b, pt );
        val -= ((double)pt.getx() * pt.getx() + (double)pt.gety() * pt.gety()) * triangleArea( a, b, c );

        return val > eps ? 1 : val < -eps ? -1 : 0;
    }
    void swapEdges( int edge )
    {
        int sedge = symEdge(edge);
        int a = getEdge(edge, PREV_AROUND_ORG);
        int b = getEdge(sedge, PREV_AROUND_ORG);

        splice(edge, a);
        splice(sedge, b);

        setEdgePoints(edge, edgeDst(a), edgeDst(b));

        splice(edge, getEdge(a, NEXT_AROUND_LEFT));
        splice(sedge, getEdge(b, NEXT_AROUND_LEFT));
    }
    int nextEdge(int edge) const
    {
        //CV_DbgAssert((size_t)(edge >> 2) < qedges.size());
        return qedges[edge >> 2].next[edge & 3];
    }
    static double triangleArea( const af2d::point<T>& a, const af2d::point<T>& b, const af2d::point<T>& c )
    {
        return ((double)b.getx() - a.getx()) * ((double)c.gety() - a.gety()) - ((double)b.gety() - a.gety()) * ((double)c.getx() - a.getx());
    }
};

}
