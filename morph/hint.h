#pragma once

#include "morphcomponent.h"

class hint
{
public:
	enum type{t_renderflag,
			  t_dib,
			  t_frame,t_frames,
			  t_erase,t_pushback,t_triangulatebkgnd,
			  t_link,
			  t_select,
			  t_new,t_load};
	
	hint(type t,const int nComponents=0,afmorph::primitive *pPrim=nullptr):m_pPrim(pPrim),m_Type(t),m_nComponents(nComponents){}
	~hint(){}

	const type m_Type;
	const int m_nComponents;
	afmorph::primitive *m_pPrim;
};
