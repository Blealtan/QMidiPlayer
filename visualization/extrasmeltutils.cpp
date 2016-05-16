#include <cstdarg>
#include <ctime>
#include <algorithm>
#include "extrasmeltutils.hpp"
SMELT* smEntity3DBuffer::sm=NULL;
SMELT* smParticle::sm=NULL;
SMELT* smParticleSystem::sm=NULL;
smVertex makeVertex(float x,float y,float z,DWORD color,float tx,float ty)
{smVertex v;v.x=x;v.y=y;v.z=z;v.col=color;v.tx=tx;v.ty=ty;return v;}
void smEntity3D::addVerices(int n,...)
{
	va_list vl;va_start(vl,n);
	for(int i=0;i<n;++i)
	{
		smVertex v=va_arg(vl,smVertex);
		vertices.push_back(v);
	}
	va_end(vl);
}
void smEntity3D::addIndices(int n,...)
{
	va_list vl;va_start(vl,n);
	for(int i=0;i<n;++i)
	{
		int idx=va_arg(vl,int);
		indices.push_back((WORD)idx);
	}
	va_end(vl);
}
smEntity3D smEntity3D::cube(smvec3d a,smvec3d b,DWORD color)
{
	//a: top left corner b: bottom right corner
	smEntity3D ret;
	ret.addVerices(8,
		makeVertex(a.x,a.y,a.z,color,0,0),makeVertex(b.x,a.y,a.z,color,0,0),
		makeVertex(b.x,b.y,a.z,color,0,0),makeVertex(a.x,b.y,a.z,color,0,0),
		makeVertex(a.x,a.y,b.z,color,0,0),makeVertex(b.x,a.y,b.z,color,0,0),
		makeVertex(b.x,b.y,b.z,color,0,0),makeVertex(a.x,b.y,b.z,color,0,0));
	ret.addIndices(36,
		0,1,3,1,2,3, 4,5,7,5,6,7,
		0,3,7,0,4,7, 1,2,6,1,5,6,
		2,3,7,2,6,7, 0,1,4,1,4,5);
	return ret;
}
smEntity3DBuffer::smEntity3DBuffer()
{
	sm=smGetInterface(SMELT_APILEVEL);
	vertices.clear();indices.clear();
}
void smEntity3DBuffer::addTransformedEntity(smEntity3D *entity,smMatrix t,smvec3d p)
{
	if(entity->vertices.size()+vertices.size()>4000)drawBatch();
	for(unsigned i=0;i<entity->indices.size();++i)
	indices.push_back(entity->indices[i]+vertices.size());
	for(unsigned i=0;i<entity->vertices.size();++i)
	{
		smvec3d tp=smvec3d(entity->vertices[i].x,entity->vertices[i].y,entity->vertices[i].z);
		tp=t*tp;tp=tp+p;vertices.push_back(makeVertex(tp.x,tp.y,tp.z,entity->vertices[i].col,entity->vertices[i].tx,entity->vertices[i].ty));
	}
}
void smEntity3DBuffer::drawBatch()
{
	sm->smDrawCustomIndexedVertices(&vertices[0],&indices[0],vertices.size(),indices.size(),BLEND_ALPHABLEND,0);
	vertices.clear();indices.clear();
}
smParticle::smParticle()
{sm=smGetInterface(SMELT_APILEVEL);dead=false;lifespan=0;}
smParticle::~smParticle(){sm->smRelease();}
void smParticle::render()
{sm->smRenderQuad(&q);}
void smParticle::update()
{
	clifespan+=sm->smGetDelta();if(clifespan>lifespan){dead=true;return;}
	vel=vel+accel;pos=pos+vel;rotv=rotv+rota;rot=rot+rotv;
	size=clifespan/lifespan*(finalsize-initsize)+initsize;
	color=ARGB(
		(DWORD)(clifespan/lifespan*(GETA(finalcolor)-GETA(initcolor)+GETA(initcolor))),
		(DWORD)(clifespan/lifespan*(GETR(finalcolor)-GETR(initcolor)+GETR(initcolor))),
		(DWORD)(clifespan/lifespan*(GETG(finalcolor)-GETG(initcolor)+GETG(initcolor))),
		(DWORD)(clifespan/lifespan*(GETB(finalcolor)-GETB(initcolor)+GETB(initcolor))));
	for(int i=0;i<4;++i)q.v[i].col=color;
	smMatrix m;m.loadIdentity();m.rotate(rot.x,1,0,0);m.rotate(rot.y,0,1,0);m.rotate(rot.z,0,0,1);
	smvec3d v0=m*smvec3d(-size,-size,0),v1=m*smvec3d(size,-size,0);
	smvec3d v2=m*smvec3d(size,size,0),v3=m*smvec3d(-size,size,0);
	q.v[0].x=v0.x;q.v[0].y=v0.y;q.v[0].z=v0.z;
	q.v[1].x=v1.x;q.v[1].y=v1.y;q.v[1].z=v1.z;
	q.v[2].x=v2.x;q.v[2].y=v2.y;q.v[2].z=v2.z;
	q.v[3].x=v3.x;q.v[3].y=v3.y;q.v[3].z=v3.z;
}
smParticleSystem::smParticleSystem()
{sm=smGetInterface(SMELT_APILEVEL);particles.clear();posGenerator=NULL;}
smParticleSystem::~smParticleSystem()
{for(unsigned i=0;i<particles.size();++i)delete particles[i];particles.clear();}
void smParticleSystem::setParticleSystemInfo(smParticleSystemInfo _psinfo)
{psinfo=_psinfo;}
void smParticleSystem::setPos(smvec3d _pos){pos=_pos;}
void smParticleSystem::setPSEmissionPosGen(smPSEmissionPositionGenerator *_gen)
{posGenerator=_gen;}
void smParticleSystem::startPS()
{active=true;nemdelay=0;re.setSeed(time(NULL));}
void smParticleSystem::stopPS()
{active=false;}
void smParticleSystem::updatePS()
{
	cemdelay+=sm->smGetDelta();
	if(cemdelay>nemdelay)
	{
		int ec=re.nextInt(psinfo.emissioncount-psinfo.ecvar,psinfo.emissioncount+psinfo.ecvar);
		for(int i=0;i<ec;++i)
		{
			smParticle *p=new smParticle();
			p->pos=pos+(posGenerator?posGenerator->genPos():smvec3d(0,0,0));
			p->rot=smvec3d(0,0,0);
			p->lifespan=re.nextDouble(psinfo.lifespan-psinfo.lifespanvar,psinfo.lifespan+psinfo.lifespanvar);
			p->initsize=re.nextDouble(psinfo.initsize-psinfo.initsizevar,psinfo.initsize+psinfo.initsizevar);
			p->finalsize=re.nextDouble(psinfo.finalsize-psinfo.finalsizevar,psinfo.finalsize+psinfo.finalsizevar);
			p->size=p->initsize;
			p->initcolor=ARGB(
				re.nextInt(GETA(psinfo.initcolor)-GETA(psinfo.initcolorvar),GETA(psinfo.initcolor)+GETA(psinfo.initcolorvar)),
				re.nextInt(GETR(psinfo.initcolor)-GETR(psinfo.initcolorvar),GETR(psinfo.initcolor)+GETR(psinfo.initcolorvar)),
				re.nextInt(GETG(psinfo.initcolor)-GETG(psinfo.initcolorvar),GETG(psinfo.initcolor)+GETG(psinfo.initcolorvar)),
				re.nextInt(GETB(psinfo.initcolor)-GETB(psinfo.initcolorvar),GETB(psinfo.initcolor)+GETB(psinfo.initcolorvar)));
			p->finalcolor=ARGB(
				re.nextInt(GETA(psinfo.finalcolor)-GETA(psinfo.finalcolorvar),GETA(psinfo.finalcolor)+GETA(psinfo.finalcolorvar)),
				re.nextInt(GETR(psinfo.finalcolor)-GETR(psinfo.finalcolorvar),GETR(psinfo.finalcolor)+GETR(psinfo.finalcolorvar)),
				re.nextInt(GETG(psinfo.finalcolor)-GETG(psinfo.finalcolorvar),GETG(psinfo.finalcolor)+GETG(psinfo.finalcolorvar)),
				re.nextInt(GETB(psinfo.finalcolor)-GETB(psinfo.finalcolorvar),GETB(psinfo.finalcolor)+GETB(psinfo.finalcolorvar)));
			p->color=p->initcolor;p->q.tex=psinfo.texture;p->q.blend=psinfo.blend;
			p->q.v[0].tx=p->q.v[3].tx=0;p->q.v[0].ty=p->q.v[1].ty=0;
			p->q.v[1].tx=p->q.v[2].tx=1;p->q.v[2].ty=p->q.v[3].ty=1;
			particles.push_back(p);
		}
		cemdelay=0;
		nemdelay=re.nextDouble(psinfo.emissiondelay-psinfo.edvar,psinfo.emissiondelay+psinfo.edvar);
	}
	for(unsigned i=0,j;i<particles.size()&&!particles[i]->dead;++i)
	{
		particles[i]->update();
		if(particles[i]->dead)
		{
			for(j=particles.size()-1;j>i&&particles[j]->dead;--j);
			std::swap(particles[i],particles[j]);
		}
	}
	while(!particles.empty()&&particles.back()->dead)
	{delete particles.back();particles.back()=NULL;particles.pop_back();}
}
void smParticleSystem::renderPS()
{for(unsigned i=0;i<particles.size();++i)particles[i]->render();}
