/*==============================================================================
 
 leaf-vocoder.c
 Created: 20 Jan 2017 12:01:54pm
 Author:  Michael R Mulshine
 
 ==============================================================================*/

#if _WIN32 || _WIN64

#include "..\Inc\leaf-effects.h"
#include "..\leaf.h"

#else

#include "../Inc/leaf-effects.h"
#include "../leaf.h"

#endif



//============================================================================================================
// TALKBOX
//============================================================================================================

//LPC vocoder adapted from MDA's excellent open source talkbox plugin code

#define ORD_MAX           34 // Was 100.
// order is defined by the set_quality function.
// it's set to max out at 0.0005 of sample rate (if you don't go above 1.0f in the quality setting) == at 48000 that's 24.
// -JS


void tTalkbox_init (tTalkbox* const voc, int bufsize)
{
    tTalkbox_initToPool(voc, bufsize, &leaf.mempool);
}

void tTalkbox_initToPool (tTalkbox* const voc, int bufsize, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tTalkbox* v = *voc = (_tTalkbox*) mpool_alloc(sizeof(_tTalkbox), m);
    v->mempool = m;
    
    v->param[0] = 0.5f;  //wet
    v->param[1] = 0.0f;  //dry
    v->param[2] = 0; // Swap
    v->param[3] = 1.0f;  //quality
    v->warpFactor = 0.0f;
    v->warpOn = 0;
    v->bufsize = bufsize;
    v->freeze = 0;
    v->G = 0.0f;
    v->car0 =   (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->car1 =   (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->window = (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->buf0 =   (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->buf1 =   (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    
    v->dl = (double*) mpool_alloc(sizeof(double) * v->bufsize, m);
    v->Rt = (double*) mpool_alloc(sizeof(double) * v->bufsize, m);

    v->k = (float*) mpool_alloc(sizeof(float) * ORD_MAX, m);

    tTalkbox_update(voc);
    tTalkbox_suspend(voc);
}

void tTalkbox_free (tTalkbox* const voc)
{
    _tTalkbox* v = *voc;
    
    mpool_free((char*)v->buf1, v->mempool);
    mpool_free((char*)v->buf0, v->mempool);
    mpool_free((char*)v->window, v->mempool);
    mpool_free((char*)v->car1, v->mempool);
    mpool_free((char*)v->car0, v->mempool);
    
    mpool_free((char*)v->dl, v->mempool);
    mpool_free((char*)v->Rt, v->mempool);
    mpool_free((char*)v->k, v->mempool);
    mpool_free((char*)v, v->mempool);
}

void tTalkbox_update(tTalkbox* const voc) ///update internal parameters...
{
    _tTalkbox* v = *voc;
    
    float fs = leaf.sampleRate;
    if(fs <  8000.0f) fs =  8000.0f;
    if(fs > 96000.0f) fs = 96000.0f;
    
    int32_t n = (int32_t)(0.01633f * fs); //this sets the window time to 16ms if the buffer is large enough. Buffer needs to be at least 784 samples at 48000
    if(n > v->bufsize) n = v->bufsize;
    
    //O = (VstInt32)(0.0005f * fs);
    v->O = (int32_t)((0.0001f + 0.0004f * v->param[3]) * fs);
    
    if(n != v->N) //recalc hanning window
    {
        v->N = n;
        float dp = TWO_PI / v->N;
        float p = 0.0f;
        for(n=0; n<v->N; n++)
        {
            v->window[n] = 0.5f - 0.5f * cosf(p);
            p += dp;
        }
    }
    v->wet = 0.5f * v->param[0] * v->param[0];
    v->dry = 2.0f * v->param[1] * v->param[1];
}

void tTalkbox_suspend(tTalkbox* const voc) ///clear any buffers...
{
    _tTalkbox* v = *voc;
    
    v->pos = v->K = 0;
    v->emphasis = 0.0f;
    v->FX = 0;
    
    v->u0 = v->u1 = v->u2 = v->u3 = v->u4 = 0.0f;
    v->d0 = v->d1 = v->d2 = v->d3 = v->d4 = 0.0f;
    
    for (int32_t i = 0; i < v->bufsize; i++)
    {
        v->buf0[i] = 0;
        v->buf1[i] = 0;
        v->car0[i] = 0;
        v->car1[i] = 0;
    }
}

// warped autocorrelation adapted from ten.enegatum@liam's post on music-dsp 2004-04-07 09:37:51
//find the order-P autocorrelation array, R, for the sequence x of length L and warping of lambda
//wAutocorrelate(&pfSrc[stIndex],siglen,R,P,0);
void tTalkbox_warpedAutocorrelate(float * x, double* dl, double* Rt, unsigned int L, float * R, unsigned int P, float lambda)
{
    double r1,r2,r1t;
    R[0]=0;
    Rt[0]=0;
    r1=0;
    r2=0;
    r1t=0;
    for(uint32_t m=0; m<L;m++)
    {
                    Rt[0] += (double)(x[m]) * (double)(x[m]);

                    dl[m]= r1 - (double)(lambda) * (double)(x[m]-r2);
                    r1 = x[m];
                    r2 = dl[m];
    }
    for(uint32_t i=1; i<=P; i++)
    {
            Rt[i]=0;
            r1=0;
            r2=0;
            for(unsigned int m=0; m<L;m++)
            {
                    Rt[i] += (double) (dl[m]) * (double)(x[m]);

                    r1t = dl[m];
                    dl[m]= r1 - (double)(lambda) * (double)(r1t-r2);
                    r1 = r1t;
                    r2 = dl[m];
            }
    }
    for(uint32_t i=0; i<=P; i++)
    {
            R[i]=(float)(Rt[i]);
    }

}

void tTalkbox_lpcDurbin(float *r, int p, float *k, float *g)
{
    int i, j;
    float a[ORD_MAX], at[ORD_MAX], e=r[0];

    for(i=0; i<=p; i++)
    {
    	a[i] = 0.0f; //probably don't need to clear at[] or k[]
    }
	k[0] = 0.0f;
    at[0] = 0.0f;
    for(i=1; i<=p; i++)
    {
        k[i] = -r[i];

        for(j=1; j<i; j++)
        {
            at[j] = a[j];
            k[i] -= a[j] * r[i-j];
        }
        if(fabs(e) < 1.0e-20f) { e = 0.0f;  break; }
        k[i] /= e;

        a[i] = k[i];
        for(j=1; j<i; j++) a[j] = at[j] + k[i] * at[i-j];

        e *= 1.0f - k[i] * k[i];
    }

    if(e < 1.0e-20f) e = 0.0f;
    *g = sqrtf(e);
}

float tTalkbox_tick(tTalkbox* const voc, float synth, float voice)
{
    _tTalkbox* v = *voc;
    
    int32_t  p0=v->pos, p1 = (v->pos + v->N/2) % v->N;
    float e=v->emphasis, w, o, x, fx=v->FX;
    float p, q, h0=0.3f, h1=0.77f;
    
    o = voice;
    x = synth;
    


    p = v->d0 + h0 *  x; v->d0 = v->d1;  v->d1 = x  - h0 * p;
    q = v->d2 + h1 * v->d4; v->d2 = v->d3;  v->d3 = v->d4 - h1 * q;
    v->d4 = x;
    x = p + q;
    
    if(v->K++)
    {
        v->K = 0;
        
        v->car0[p0] = v->car1[p1] = x; //carrier input
        
        x = o - e;  e = o;  //6dB/oct pre-emphasis
        
        w = v->window[p0]; fx = v->buf0[p0] * w;  v->buf0[p0] = x * w;  //50% overlapping hanning windows
        if(++p0 >= v->N) { tTalkbox_lpc(v->buf0, v->car0, v->dl, v->Rt, v->N, v->O, v->warpFactor, v->warpOn, v->k, v->freeze, &v->G);  p0 = 0; }
        
        w = 1.0f - w;  fx += v->buf1[p1] * w;  v->buf1[p1] = x * w;
        if(++p1 >= v->N) { tTalkbox_lpc(v->buf1, v->car1, v->dl, v->Rt, v->N, v->O, v->warpFactor, v->warpOn, v->k, v->freeze, &v->G);  p1 = 0; }
    }
    
    p = v->u0 + h0 * fx; v->u0 = v->u1;  v->u1 = fx - h0 * p;
    q = v->u2 + h1 * v->u4; v->u2 = v->u3;  v->u3 = v->u4 - h1 * q;
    v->u4 = fx;
    x = p + q;
    
    o = x;
    
    v->emphasis = e;
    v->pos = p0;
    v->FX = fx;
    

    return o;
}


void tTalkbox_lpc(float *buf, float *car, double* dl, double* Rt, int32_t n, int32_t o, float warp, int warpOn, float *k, int freeze, float *G)
{
    float z[ORD_MAX], r[ORD_MAX], x;
    int32_t i, j, nn=n;


	if (warpOn == 0)
	{
		for(j=0; j<=o; j++, nn--)  //buf[] is already emphasized and windowed
		{
			z[j] = r[j] = 0.0f;
			for(i=0; i<nn; i++) r[j] += buf[i] * buf[i+j]; //autocorrelation
		}
	}
	else
	{
		for(j=0; j<=o; j++, nn--)  //buf[] is already emphasized and windowed
		{
			z[j] = r[j] = 0.0f;
		}
		tTalkbox_warpedAutocorrelate(buf, dl, Rt, n, r, o, warp);
	}

	r[0] *= 1.001f;  //stability fix

	float min = 0.000001f;
	if (!freeze)
	{
		if(r[0] < min)
		{
			for(i=0; i<n; i++)
			{
				buf[i] = 0.0f;
				return;
			}
		}

		tTalkbox_lpcDurbin(r, o, k, G);  //calc reflection coeffs

		//this is for stability to keep reflection coefficients inside the unit circle
		//in mda's code it's .995 but in Harma's papers I've seen 0.998.  just needs to be less than 1 it seems but maybe some wiggle room to avoid instability from floating point precision -JS
		for(i=0; i<=o; i++)
		{
			if(k[i] > 0.998f) k[i] = 0.998f; else if(k[i] < -0.998f) k[i] = -.998f;
		}
    }
    for(i=0; i<n; i++)
    {
        x = G[0] * car[i];
        for(j=o; j>0; j--)  //lattice filter
        {
            x -= k[j] * z[j-1];
            z[j] = z[j-1] + k[j] * x;
        }
        buf[i] = z[0] = x;  //output buf[] will be windowed elsewhere
    }
}

void tTalkbox_setQuality(tTalkbox* const voc, float quality)
{
    _tTalkbox* v = *voc;
    v->param[3] = quality;
    v->O = (int32_t)((0.0001f + 0.0004f * v->param[3]) * leaf.sampleRate);
    if (v->O >= ORD_MAX)
    {
    	v->O = ORD_MAX-1;
    }
}

void tTalkbox_setWarpFactor(tTalkbox* const voc, float warpFactor)
{
    _tTalkbox* v = *voc;

    v->warpFactor = warpFactor;
}

void tTalkbox_setWarpOn(tTalkbox* const voc, float warpOn)
{
    _tTalkbox* v = *voc;

    v->warpOn = warpOn;
}

void tTalkbox_setFreeze(tTalkbox* const voc, float freeze)
{
    _tTalkbox* v = *voc;

    v->freeze = freeze;
}



////

// LPC vocoder adapted from MDA's excellent open source talkbox plugin code
// order is defined by the set_quality function.
// it's set to max out at 0.0005 of sample rate (if you don't go above 1.0f in the quality setting) == at 48000 that's 24.

//the "float" version has no double calculations for faster computation on single-precision FPUs

// -JS


void tTalkboxFloat_init (tTalkboxFloat* const voc, int bufsize)
{
    tTalkboxFloat_initToPool(voc, bufsize, &leaf.mempool);
}

void tTalkboxFloat_initToPool (tTalkboxFloat* const voc, int bufsize, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tTalkboxFloat* v = *voc = (_tTalkboxFloat*) mpool_alloc(sizeof(_tTalkboxFloat), m);
    v->mempool = m;

    v->param[0] = 0.5f;  //wet
    v->param[1] = 0.0f;  //dry
    v->param[2] = 0; // Swap
    v->param[3] = 1.0f;  //quality
    v->warpFactor = 0.0f;
    v->warpOn = 0;
    v->bufsize = bufsize;
    v->freeze = 0;
    v->G = 0.0f;
    v->car0 =   (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->car1 =   (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->window = (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->buf0 =   (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->buf1 =   (float*) mpool_alloc(sizeof(float) * v->bufsize, m);

    v->dl = (float*) mpool_alloc(sizeof(float) * v->bufsize, m);
    v->Rt = (float*) mpool_alloc(sizeof(float) * v->bufsize, m);

    v->k = (float*) mpool_alloc(sizeof(float) * ORD_MAX, m);

    tTalkboxFloat_update(voc);
    tTalkboxFloat_suspend(voc);
}

void tTalkboxFloat_free (tTalkboxFloat* const voc)
{
    _tTalkboxFloat* v = *voc;

    mpool_free((char*)v->buf1, v->mempool);
    mpool_free((char*)v->buf0, v->mempool);
    mpool_free((char*)v->window, v->mempool);
    mpool_free((char*)v->car1, v->mempool);
    mpool_free((char*)v->car0, v->mempool);

    mpool_free((char*)v->dl, v->mempool);
    mpool_free((char*)v->Rt, v->mempool);
    mpool_free((char*)v->k, v->mempool);
    mpool_free((char*)v, v->mempool);
}

void tTalkboxFloat_update(tTalkboxFloat* const voc) ///update internal parameters...
{
    _tTalkboxFloat* v = *voc;

    float fs = leaf.sampleRate;
    if(fs <  8000.0f) fs =  8000.0f;
    if(fs > 96000.0f) fs = 96000.0f;

    int32_t n = (int32_t)(0.01633f * fs); //this sets the window time to 16ms if the buffer is large enough. Buffer needs to be at least 784 samples at 48000
    if(n > v->bufsize) n = v->bufsize;

    //O = (VstInt32)(0.0005f * fs);
    v->O = (int32_t)((0.0001f + 0.0004f * v->param[3]) * fs);

    if(n != v->N) //recalc hanning window
    {
        v->N = n;
        float dp = TWO_PI / v->N;
        float p = 0.0f;
        for(n=0; n<v->N; n++)
        {
            v->window[n] = 0.5f - 0.5f * cosf(p);
            p += dp;
        }
    }
    v->wet = 0.5f * v->param[0] * v->param[0];
    v->dry = 2.0f * v->param[1] * v->param[1];
}

void tTalkboxFloat_suspend(tTalkboxFloat* const voc) ///clear any buffers...
{
    _tTalkboxFloat* v = *voc;

    v->pos = v->K = 0;
    v->emphasis = 0.0f;
    v->FX = 0;

    v->u0 = v->u1 = v->u2 = v->u3 = v->u4 = 0.0f;
    v->d0 = v->d1 = v->d2 = v->d3 = v->d4 = 0.0f;

    for (int32_t i = 0; i < v->bufsize; i++)
    {
        v->buf0[i] = 0;
        v->buf1[i] = 0;
        v->car0[i] = 0;
        v->car1[i] = 0;
    }
}

// warped autocorrelation adapted from ten.enegatum@liam's post on music-dsp 2004-04-07 09:37:51
//find the order-P autocorrelation array, R, for the sequence x of length L and warping of lambda
//wAutocorrelate(&pfSrc[stIndex],siglen,R,P,0);
void tTalkboxFloat_warpedAutocorrelate(float * x, float* dl, float* Rt, unsigned int L, float * R, unsigned int P, float lambda)
{
    float r1,r2,r1t;
    R[0]=0;
    Rt[0]=0;
    r1=0;
    r2=0;
    r1t=0;
    for(uint32_t m=0; m<L;m++)
    {
                    Rt[0] += (x[m]) * (x[m]);

                    dl[m]= r1 - lambda * (x[m]-r2);
                    r1 = x[m];
                    r2 = dl[m];
    }
    for(uint32_t i=1; i<=P; i++)
    {
            Rt[i]=0;
            r1=0;
            r2=0;
            for(unsigned int m=0; m<L;m++)
            {
                    Rt[i] +=  (dl[m]) * (x[m]);

                    r1t = dl[m];
                    dl[m]= r1 - lambda * (r1t-r2);
                    r1 = r1t;
                    r2 = dl[m];
            }
    }
    for(uint32_t i=0; i<=P; i++)
    {
            R[i]=Rt[i];
    }

}

void tTalkboxFloat_lpcDurbin(float *r, int p, float *k, float *g)
{
    int i, j;
    float a[ORD_MAX], at[ORD_MAX], e=r[0];

    for(i=0; i<=p; i++)
    {
    	a[i] = 0.0f; //probably don't need to clear at[]

    }
	k[0] = 0.0f;
    at[0] = 0.0f;
    for(i=1; i<=p; i++)
    {
        k[i] = -r[i];

        for(j=1; j<i; j++)
        {
            at[j] = a[j];
            k[i] -= a[j] * r[i-j];
        }
        if(fabs(e) < 1.0e-20f) { e = 0.0f;  break; }
        k[i] /= e;

        a[i] = k[i];
        for(j=1; j<i; j++) a[j] = at[j] + k[i] * at[i-j];

        e *= 1.0f - k[i] * k[i];
    }

    if(e < 1.0e-20f) e = 0.0f;
    *g = sqrtf(e);
}

float tTalkboxFloat_tick(tTalkboxFloat* const voc, float synth, float voice)
{
    _tTalkboxFloat* v = *voc;

    int32_t  p0=v->pos, p1 = (v->pos + v->N/2) % v->N;
    float e=v->emphasis, w, o, x, fx=v->FX;
    float p, q, h0=0.3f, h1=0.77f;

    o = voice;
    x = synth;



    p = v->d0 + h0 *  x; v->d0 = v->d1;  v->d1 = x  - h0 * p;
    q = v->d2 + h1 * v->d4; v->d2 = v->d3;  v->d3 = v->d4 - h1 * q;
    v->d4 = x;
    x = p + q;

    if(v->K++)
    {
        v->K = 0;

        v->car0[p0] = v->car1[p1] = x; //carrier input

        x = o - e;  e = o;  //6dB/oct pre-emphasis

        w = v->window[p0]; fx = v->buf0[p0] * w;  v->buf0[p0] = x * w;  //50% overlapping hanning windows
        if(++p0 >= v->N) { tTalkboxFloat_lpc(v->buf0, v->car0, v->dl, v->Rt, v->N, v->O, v->warpFactor, v->warpOn, v->k, v->freeze, &v->G);  p0 = 0; }

        w = 1.0f - w;  fx += v->buf1[p1] * w;  v->buf1[p1] = x * w;
        if(++p1 >= v->N) { tTalkboxFloat_lpc(v->buf1, v->car1, v->dl, v->Rt, v->N, v->O, v->warpFactor, v->warpOn, v->k, v->freeze, &v->G);  p1 = 0; }
    }

    p = v->u0 + h0 * fx; v->u0 = v->u1;  v->u1 = fx - h0 * p;
    q = v->u2 + h1 * v->u4; v->u2 = v->u3;  v->u3 = v->u4 - h1 * q;
    v->u4 = fx;
    x = p + q;

    o = x;

    v->emphasis = e;
    v->pos = p0;
    v->FX = fx;


    return o;
}


void tTalkboxFloat_lpc(float *buf, float *car, float* dl, float* Rt, int32_t n, int32_t o, float warp, int warpOn, float *k, int freeze, float *G)
{
    float z[ORD_MAX], r[ORD_MAX], x;
    int32_t i, j, nn=n;


	if (warpOn == 0)
	{
		for(j=0; j<=o; j++, nn--)  //buf[] is already emphasized and windowed
		{
			z[j] = r[j] = 0.0f;
			for(i=0; i<nn; i++) r[j] += buf[i] * buf[i+j]; //autocorrelation
		}
	}
	else
	{
		for(j=0; j<=o; j++, nn--)  //buf[] is already emphasized and windowed
		{
			z[j] = r[j] = 0.0f;
		}
		tTalkboxFloat_warpedAutocorrelate(buf, dl, Rt, n, r, o, warp);
	}

	r[0] *= 1.001f;  //stability fix

	float min = 0.000001f;
	if (!freeze)
	{
		if(r[0] < min)
		{
			for(i=0; i<n; i++)
			{
				buf[i] = 0.0f;
				return;
			}
		}

		tTalkbox_lpcDurbin(r, o, k, G);  //calc reflection coeffs

		//this is for stability to keep reflection coefficients inside the unit circle
		//but in Harma's papers I've seen 0.998.  just needs to be less than 1 it seems but maybe some wiggle room to avoid instability from floating point precision -JS
		for(i=0; i<=o; i++)
		{
			if(k[i] > 0.998f) k[i] = 0.998f; else if(k[i] < -0.998f) k[i] = -.998f;
		}
    }
    for(i=0; i<n; i++)
    {
        x = G[0] * car[i];
        for(j=o; j>0; j--)  //lattice filter
        {
            x -= k[j] * z[j-1];
            z[j] = z[j-1] + k[j] * x;
        }
        buf[i] = z[0] = x;  //output buf[] will be windowed elsewhere
    }
}

void tTalkboxFloat_setQuality(tTalkboxFloat* const voc, float quality)
{
    _tTalkboxFloat* v = *voc;
    v->param[3] = quality;
    v->O = (int32_t)((0.0001f + 0.0004f * v->param[3]) * leaf.sampleRate);
    if (v->O >= ORD_MAX)
    {
    	v->O = ORD_MAX-1;
    }
}

void tTalkboxFloat_setWarpFactor(tTalkboxFloat* const voc, float warpFactor)
{
    _tTalkboxFloat* v = *voc;

    v->warpFactor = warpFactor;
}

void tTalkboxFloat_setWarpOn(tTalkboxFloat* const voc, float warpOn)
{
    _tTalkboxFloat* v = *voc;

    v->warpOn = warpOn;
}

void tTalkboxFloat_setFreeze(tTalkboxFloat* const voc, float freeze)
{
    _tTalkboxFloat* v = *voc;

    v->freeze = freeze;
}


//============================================================================================================
// VOCODER
//============================================================================================================

void tVocoder_init (tVocoder* const voc)
{
    tVocoder_initToPool(voc, &leaf.mempool);
}

void tVocoder_initToPool (tVocoder* const voc, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tVocoder* v = *voc = (_tVocoder*) mpool_alloc(sizeof(_tVocoder), m);
    v->mempool = m;
    
    v->param[0] = 0.33f;  //input select
    v->param[1] = 0.50f;  //output dB
    v->param[2] = 0.40f;  //hi thru
    v->param[3] = 0.40f;  //hi band
    v->param[4] = 0.16f;  //envelope
    v->param[5] = 0.55f;  //filter q
    v->param[6] = 0.6667f;//freq range
    v->param[7] = 0.33f;  //num bands
    
    tVocoder_update(voc);
}

void tVocoder_free (tVocoder* const voc)
{
    _tVocoder* v = *voc;
    
    mpool_free((char*)v, v->mempool);
}

void        tVocoder_update      (tVocoder* const voc)
{
    _tVocoder* v = *voc;
    
    float tpofs = 6.2831853f * leaf.invSampleRate;
    
    float rr, th;
    
    float sh;
    
    int32_t i;
    
    v->gain = (float)pow(10.0f, 2.0f * v->param[1] - 3.0f * v->param[5] - 2.0f);
    
    v->thru = (float)pow(10.0f, 0.5f + 2.0f * v->param[1]);
    v->high =  v->param[3] * v->param[3] * v->param[3] * v->thru;
    v->thru *= v->param[2] * v->param[2] * v->param[2];
    
    if(v->param[7]<0.5f)
    {
        v->nbnd=8;

        v->f[1][2] = 3000.0f;
        v->f[2][2] = 2200.0f;
        v->f[3][2] = 1500.0f;
        v->f[4][2] = 1080.0f;
        v->f[5][2] = 700.0f;
        v->f[6][2] = 390.0f;
        v->f[7][2] = 190.0f;
    }
    else
    {
        v->nbnd=16;

        v->f[ 1][2] = 5000.0f; //+1000
        v->f[ 2][2] = 4000.0f; //+750
        v->f[ 3][2] = 3250.0f; //+500
        v->f[ 4][2] = 2750.0f; //+450
        v->f[ 5][2] = 2300.0f; //+300
        v->f[ 6][2] = 2000.0f; //+250
        v->f[ 7][2] = 1750.0f; //+250
        v->f[ 8][2] = 1500.0f; //+250
        v->f[ 9][2] = 1250.0f; //+250
        v->f[10][2] = 1000.0f; //+250
        v->f[11][2] =  750.0f; //+210
        v->f[12][2] =  540.0f; //+190
        v->f[13][2] =  350.0f; //+155
        v->f[14][2] =  195.0f; //+100
        v->f[15][2] =   95.0f;
    }
    
    if(v->param[4]<0.05f) //freeze
    {
        for(i=0;i<v->nbnd;i++) v->f[i][12]=0.0f;
    }
    else
    {
        v->f[0][12] = (float)pow(10.0, -1.7 - 2.7f * v->param[4]); //envelope speed
        
        rr = 0.022f / (float)v->nbnd; //minimum proportional to frequency to stop distortion
        for(i=1;i<v->nbnd;i++)
        {
            v->f[i][12] = (float)(0.025 - rr * (double)i);
            if(v->f[0][12] < v->f[i][12]) v->f[i][12] = v->f[0][12];
        }
        v->f[0][12] = 0.5f * v->f[0][12]; //only top band is at full rate
    }
    
    rr = 1.0 - pow(10.0f, -1.0f - 1.2f * v->param[5]);
    sh = (float)pow(2.0f, 3.0f * v->param[6] - 1.0f); //filter bank range shift
    
    for(i=1;i<v->nbnd;i++)
    {
        v->f[i][2] *= sh;
        th = acos((2.0 * rr * cos(tpofs * v->f[i][2])) / (1.0 + rr * rr));
        v->f[i][0] = (float)(2.0 * rr * cos(th)); //a0
        v->f[i][1] = (float)(-rr * rr);           //a1
        //was .98
        v->f[i][2] *= 0.96f; //shift 2nd stage slightly to stop high resonance peaks
        th = acos((2.0 * rr * cos(tpofs * v->f[i][2])) / (1.0 + rr * rr));
        v->f[i][2] = (float)(2.0 * rr * cos(th));
    }
}

float       tVocoder_tick        (tVocoder* const voc, float synth, float voice)
{
    _tVocoder* v = *voc;
    
    float a, b, o=0.0f, aa, bb, oo = v->kout, g = v->gain, ht = v->thru, hh = v->high, tmp;
    uint32_t i, k = v->kval, nb = v->nbnd;
    
    a = voice; //speech
    b = synth; //synth
    
    tmp = a - v->f[0][7]; //integrate modulator for HF band and filter bank pre-emphasis
    v->f[0][7] = a;
    a = tmp;
    
    if(tmp<0.0f) tmp = -tmp;
    v->f[0][11] -= v->f[0][12] * (v->f[0][11] - tmp);      //high band envelope
    o = v->f[0][11] * (ht * a + hh * (b - v->f[0][3])); //high band + high thru
    
    v->f[0][3] = b; //integrate carrier for HF band
    
    if(++k & 0x1) //this block runs at half sample rate
    {
        oo = 0.0f;
        aa = a + v->f[0][9] - v->f[0][8] - v->f[0][8];  //apply zeros here instead of in each reson
        v->f[0][9] = v->f[0][8];  v->f[0][8] = a;
        bb = b + v->f[0][5] - v->f[0][4] - v->f[0][4];
        v->f[0][5] = v->f[0][4];  v->f[0][4] = b;
        
        for(i=1; i<nb; i++) //filter bank: 4th-order band pass
        {
            tmp = v->f[i][0] * v->f[i][3] + v->f[i][1] * v->f[i][4] + bb;
            v->f[i][4] = v->f[i][3];
            v->f[i][3] = tmp;
            tmp += v->f[i][2] * v->f[i][5] + v->f[i][1] * v->f[i][6];
            v->f[i][6] = v->f[i][5];
            v->f[i][5] = tmp;
            
            tmp = v->f[i][0] * v->f[i][7] + v->f[i][1] * v->f[i][8] + aa;
            v->f[i][8] = v->f[i][7];
            v->f[i][7] = tmp;
            tmp += v->f[i][2] * v->f[i][9] + v->f[i][1] * v->f[i][10];
            v->f[i][10] = v->f[i][9];
            v->f[i][9] = tmp;
            
            if(tmp<0.0f) tmp = -tmp;
            v->f[i][11] -= v->f[i][12] * (v->f[i][11] - tmp);
            oo += v->f[i][5] * v->f[i][11];
        }
    }
    o += oo * g; //effect of interpolating back up to Fs would be minimal (aliasing >16kHz)
    
    v->kout = oo;
    v->kval = k & 0x1;
#ifdef NO_DENORMAL_CHECK
#else
    if(fabs(v->f[0][11])<1.0e-10) v->f[0][11] = 0.0f; //catch HF envelope denormal
    
    for(i=1;i<nb;i++)
        if(fabs(v->f[i][3])<1.0e-10 || fabs(v->f[i][7])<1.0e-10)
            for(k=3; k<12; k++) v->f[i][k] = 0.0f; //catch reson & envelope denormals
#endif
    if(fabs(o)>10.0f) tVocoder_suspend(voc); //catch instability
    
    return o;
    
}

void        tVocoder_suspend     (tVocoder* const voc)
{
    _tVocoder* v = *voc;
    
    int32_t i, j;
    
    for(i=0; i<v->nbnd; i++) for(j=3; j<12; j++) v->f[i][j] = 0.0f; //zero band filters and envelopes
    v->kout = 0.0f;
    v->kval = 0;
}


/// Glottal Pulse (Rosenberg model)

void tRosenbergGlottalPulse_init (tRosenbergGlottalPulse* const gp)
{
    tRosenbergGlottalPulse_initToPool(gp, &leaf.mempool);
}

void tRosenbergGlottalPulse_initToPool (tRosenbergGlottalPulse* const gp, tMempool* const mp)
{
     _tMempool* m = *mp;
    _tRosenbergGlottalPulse* g = *gp = (_tRosenbergGlottalPulse*) mpool_alloc(sizeof(_tRosenbergGlottalPulse), m);
    g->mempool = m;

    g->phase  = 0.0f;
    g->openLength = 0.0f;
    g->pulseLength = 0.0f;
    g->freq = 0.0f;
    g->inc = 0.0f;


}
void tRosenbergGlottalPulse_free (tRosenbergGlottalPulse* const gp)
{
    _tRosenbergGlottalPulse* g = *gp;
    mpool_free((char*)g, g->mempool);
}

float   tRosenbergGlottalPulse_tick           (tRosenbergGlottalPulse* const gp)
{
    _tRosenbergGlottalPulse* g = *gp;

    float output = 0.0f;

    // Phasor increment
    g->phase += g->inc;
    while (g->phase >= 1.0f) g->phase -= 1.0f;
    while (g->phase < 0.0f) g->phase += 1.0f;

    if (g->phase < g->openLength)
    {
        output = 0.5f*(1.0f-fastercosf(PI * g->phase));
    }

    else if (g->phase < g->pulseLength)
    {
        output = fastercosf(HALF_PI * (g->phase-g->openLength)* g->invPulseLengthMinusOpenLength);
    }

    else
    {
        output = 0.0f;
    }
    return output;
}


float   tRosenbergGlottalPulse_tickHQ           (tRosenbergGlottalPulse* const gp)
{
    _tRosenbergGlottalPulse* g = *gp;

    float output = 0.0f;

    // Phasor increment
    g->phase += g->inc;
    while (g->phase >= 1.0f) g->phase -= 1.0f;
    while (g->phase < 0.0f) g->phase += 1.0f;

    if (g->phase < g->openLength)
    {
        output = 0.5f*(1.0f-cosf(PI * g->phase));
    }

    else if (g->phase < g->pulseLength)
    {
        output = cosf(HALF_PI * (g->phase-g->openLength)* g->invPulseLengthMinusOpenLength);
    }

    else
    {
        output = 0.0f;
    }
    return output;
}

void   tRosenbergGlottalPulse_setFreq           (tRosenbergGlottalPulse* const gp, float freq)
{
    _tRosenbergGlottalPulse* g = *gp;
    g->freq = freq;
    g->inc = freq * leaf.invSampleRate;
}

void   tRosenbergGlottalPulse_setOpenLength           (tRosenbergGlottalPulse* const gp, float openLength)
{
    _tRosenbergGlottalPulse* g = *gp;
    g->openLength = openLength;
    g->invPulseLengthMinusOpenLength = 1.0f / (g->pulseLength - g->openLength);
}

void   tRosenbergGlottalPulse_setPulseLength           (tRosenbergGlottalPulse* const gp, float pulseLength)
{
    _tRosenbergGlottalPulse* g = *gp;
    g->pulseLength = pulseLength;
    g->invPulseLengthMinusOpenLength = 1.0f / (g->pulseLength - g->openLength);
}

void   tRosenbergGlottalPulse_setOpenLengthAndPulseLength           (tRosenbergGlottalPulse* const gp, float openLength, float pulseLength)
{
    _tRosenbergGlottalPulse* g = *gp;
    g->openLength = openLength;
    g->pulseLength = pulseLength;
    g->invPulseLengthMinusOpenLength = 1.0f / (g->pulseLength - g->openLength);
}



//============================================================================================================
// SOLAD
//============================================================================================================
/******************************************************************************/
/***************** static function declarations *******************************/
/******************************************************************************/

static void solad_init(_tSOLAD *w);
static inline float read_sample(_tSOLAD *w, float floatindex);
static void pitchdown(_tSOLAD *w, float *out);
static void pitchup(_tSOLAD *w, float *out);

/******************************************************************************/
/***************** public access functions ************************************/
/******************************************************************************/

// init
void tSOLAD_init (tSOLAD* const wp)
{
    tSOLAD_initToPool(wp, &leaf.mempool);
}

void tSOLAD_initToPool (tSOLAD* const wp, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tSOLAD* w = *wp = (_tSOLAD*) mpool_calloc(sizeof(_tSOLAD), m);
    w->mempool = m;
    
    w->pitchfactor = 1.;
    w->delaybuf = (float*) mpool_calloc(sizeof(float) * (LOOPSIZE+16), m);

    solad_init(w);
}

void tSOLAD_free (tSOLAD* const wp)
{
    _tSOLAD* w = *wp;
    
    mpool_free((char*)w->delaybuf, w->mempool);
    mpool_free((char*)w, w->mempool);
}

// send one block of input samples, receive one block of output samples
void tSOLAD_ioSamples(tSOLAD* const wp, float* in, float* out, int blocksize)
{
    _tSOLAD* w = *wp;
    
    int i = w->timeindex;
    int n = w->blocksize = blocksize;
    
    if(!i) w->delaybuf[LOOPSIZE] = in[0];   // copy one sample for interpolation
    while(n--) w->delaybuf[i++] = *in++;    // copy one input block to delay buffer
    
    if(w->pitchfactor > 1) pitchup(w, out);
    else pitchdown(w, out);
    
    w->timeindex += blocksize;
    w->timeindex &= LOOPMASK;
}

// set periodicity analysis data
void tSOLAD_setPeriod(tSOLAD* const wp, float period)
{
    _tSOLAD* w = *wp;
    
    if(period > MAXPERIOD) period = MAXPERIOD;
    if(period > MINPERIOD) w->period = period;  // ignore period when too small
}

// set pitch factor between 0.25 and 4
void tSOLAD_setPitchFactor(tSOLAD* const wp, float pitchfactor)
{
    _tSOLAD* w = *wp;
    
    if (pitchfactor <= 0.0f) return;
    w->pitchfactor = pitchfactor;
}

// force readpointer lag
void tSOLAD_setReadLag(tSOLAD* const wp, float readlag)
{
    _tSOLAD* w = *wp;
    
    if(readlag < 0) readlag = 0;
    if(readlag < w->readlag)               // do not jump backward, only forward
    {
        w->jump = w->readlag - readlag;
        w->readlag = readlag;
        w->xfadelength = readlag;
        w->xfadevalue = 1;
    }
}

// reset state variables
void tSOLAD_resetState(tSOLAD* const wp)
{
    _tSOLAD* w = *wp;
    
    int n = LOOPSIZE + 1;
    float *buf = w->delaybuf;
    
    while(n--) *buf++ = 0;
    solad_init(w);
}

/******************************************************************************/
/******************** private procedures **************************************/
/******************************************************************************/

/*
 Function pitchdown() is called to read samples from the delay buffer when pitch
 factor is between 0.25 and 1. The read pointer lags behind because of the slowed
 down speed, and it must jump forward towards the write pointer soon as there is
 sufficient space to jump. That is, if there is at least one period of the input
 signal between read pointer and write pointer.  When short periods follow up on
 long periods, the read pointer may have space to jump over more than one period
 lenghts. Jump length must be [periodlength ^ 2] in any case.
 
 A linear crossfade function joins the jump-from point with the jump-to point.
 The crossfade must be completed before another read pointer jump is allowed.
 Length of the crossfade function is stored as a number of samples in terms of
 the input sample rate. This length is dynamically translated
 to a crossfade length expressed in output reading rate, according to pitch
 factor which can change before the crossfade is completed. Crossfade length does
 not cover an invariable length in periods for all pitch transposition factors.
 For pitch factors from 0.5 till 1, crossfade length is stretched in the
 output just as much as the signal itself, as crossfade speed is set to equal
 pitch factor. For pitch factors below 0.5, the read pointer wants to jump
 forward before one period is read, therefore the crossfade length as expressed
 in output periods must be shorter. Crossfade speed is set to [1 - pitchfactor]
 for those cases. Pitch factor 0.5 is the natural switch point between crossfade
 speeds [pitchfactor] and [1 - pitchfactor] because 0.5 == 1 - 0.5. The crossfade
 speed modification for pitch factors below 0.5 also means that much of the
 original signal content will be skipped.
 */


static void pitchdown(_tSOLAD* const w, float *out)
{
    int n = w->blocksize;
    float refindex = (float)(w->timeindex + LOOPSIZE); // no negative values!
    float pitchfactor = w->pitchfactor;
    float period = w->period;
    float readlag = w->readlag;
    float readlagstep = 1 - pitchfactor;
    float jump = w->jump;
    float xfadevalue = w->xfadevalue;
    float xfadelength = w->xfadelength;
    float xfadespeed, xfadestep, readindex, outputsample;
    
    if(pitchfactor > 0.5) xfadespeed = pitchfactor;
    else xfadespeed = 1 - pitchfactor;
    xfadestep = xfadespeed / xfadelength;
    
    while(n--)
    {
        if(readlag > period)        // check if read pointer may jump forward...
        {
            if(xfadevalue <= 0)      // ...but do not interrupt crossfade
            {
                jump = period;                           // jump forward
                while((jump * 2) < readlag) jump *= 2;   // use available space
                readlag -= jump;                         // reduce read pointer lag
                xfadevalue = 1;                          // start crossfade
                xfadelength = period - 1;
                xfadestep = xfadespeed / xfadelength;
            }
        }
        
        readindex = refindex - readlag;
        outputsample = read_sample(w, readindex);
        
        if(xfadevalue > 0)
        {
            outputsample *= (1 - xfadevalue);                               // fadein
            outputsample += read_sample(w, readindex - jump) * xfadevalue;  // fadeout
            xfadevalue -= xfadestep;
        }
        
        *out++ = outputsample;
        refindex += 1;
        readlag += readlagstep;
    }
    
    w->jump = jump;                 // state variables
    w->readlag = readlag;
    w->xfadevalue = xfadevalue;
    w->xfadelength = xfadelength;
}


/*
 Function pitchup() for pitch factors above 1 is more complicated than
 pitchdown(). The read pointer increments faster than the write pointer and a
 backward jump must happen in time, reckoning with the crossfade region. The read
 pointer backward jump length is always one period. In order to minimize the area
 of signal duplicates, crossfade length is aimed at [period / pitchfactor].
 This leads to a crossfade speed of [pitchfactor * pitchfactor].
 
 Some samples for the fade out (but not all of them) must already be in the
 buffer, otherwise we will run out of input samples before the crossfade is
 completed. The ratio of past samples and future samples for a crossfade of any
 length is as follows:
 
 past samples: xfadelength * (1 - 1 / pitchfactor)
 future samples: xfadelength * (1 / pitchfactor)
 
 For example in the case of pitch factor 1.5 this would be:
 
 past samples: xfadelength * (1 - 1 / 1.5) = xfadelength * 1 / 3
 future samples: xfadelength * (1 / 1.5) = xfadelength * 2 / 3
 
 In the case of pitch factor 4 this would be:
 
 past samples: xfadelength * (1 - 1 / 4) = xfadelength * 3 / 4
 future samples: xfadelength * (1 / 4) = xfadelength * 1 / 4
 
 The read pointer lag must therefore preserve a minimum dependent on pitch
 factor. The minimum is called 'limit' here:
 
 limit = period * (pitchfactor - 1) / pitchfactor * pitchfactor
 
 Components of this expression are combined to reuse them in operations, while
 (pitchfactor - 1) is changed to (pitchfactor - 0.99) to avoid numerical
 resolution issues for pitch factors slightly above 1:
 
 xfadespeed = pitchfactor * pitchfactor
 limitfactor = (pitchfactor - 0.99) / xfadespeed
 limit = period * limitfactor
 
 When read lag is smaller than this limit, the read pointer must preferably
 jump backward, unless a previous crossfade is not yet completed. Crossfades must
 preferably be completed, unless the read pointer lag becomes smaller than zero.
 With fluctuating period lengths and pitch factors, the readpointer lag limit may
 change from one input block to the next in such a way that the actual lag is
 suddenly much smaller than the limit, and the intended crossfade length can not
 be applied. Therefore the crossfade length is simply calculated from the
 available amount of samples for all cases, like so:
 
 xfadelength = readlag / limitfactor
 
 For most occurrences, this will amount to a crossfade length reduced to
 [period / pitchfactor] in the output for pitch factors above 1, while in some
 cases it will be considerably shorter. Fortunately, an incidental aberration of
 the intended crossfade length hardly ever creates an audible artifact. The
 reason to specify preferred crossfade length according to pitch factor is to
 minimize the impression of echoes without sacrificing too much of the signal
 content. The readpointer jump length remains one period in any case.
 
 Sometimes, the input signal periodicity may decrease substantially between one
 signal block and the next. In such cases it may be possible for the read pointer
 to jump forward and reduce latency. For every signal block, a check on this
 possibility is done. A previous crossfade must be completed before a forward
 jump is allowed.
 */
static void pitchup(_tSOLAD* const w, float *out)
{
    int n = w->blocksize;
    float refindex = (float)(w->timeindex + LOOPSIZE); // no negative values
    float pitchfactor = w->pitchfactor;
    float period = w->period;
    float readlag = w->readlag;
    float jump = w->jump;
    float xfadevalue = w->xfadevalue;
    float xfadelength = w->xfadelength;
    
    float readlagstep = pitchfactor - 1;
    float xfadespeed = pitchfactor * pitchfactor;
    float xfadestep = xfadespeed / xfadelength;
    float limitfactor = (pitchfactor - (float)0.99) / xfadespeed;
    float limit = period * limitfactor;
    float readindex, outputsample;
    
    if((readlag > (period + 2 * limit)) & (xfadevalue < 0))
    {
        jump = period;                                        // jump forward
        while((jump * 2) < (readlag - 2 * limit)) jump *= 2;  // use available space
        readlag -= jump;                                      // reduce read pointer lag
        xfadevalue = 1;                                       // start crossfade
        xfadelength = period - 1;
        xfadestep = xfadespeed / xfadelength;
    }
    
    while(n--)
    {
        if(readlag < limit)  // check if read pointer should jump backward...
        {
            if((xfadevalue < 0) | (readlag < 0)) // ...but try not to interrupt crossfade
            {
                xfadelength = readlag / limitfactor;
                if(xfadelength < 1) xfadelength = 1;
                xfadestep = xfadespeed / xfadelength;
                
                jump = -period;         // jump backward
                readlag += period;      // increase read pointer lag
                xfadevalue = 1;         // start crossfade
            }
        }
        
        readindex = refindex - readlag;
        outputsample = read_sample(w, readindex);
        
        if(xfadevalue > 0)
        {
            outputsample *= (1 - xfadevalue);
            outputsample += read_sample(w, readindex - jump) * xfadevalue;
            xfadevalue -= xfadestep;
        }
        
        *out++ = outputsample;
        refindex += 1;
        readlag -= readlagstep;
    }
    
    w->readlag = readlag;               // state variables
    w->jump = jump;
    w->xfadelength = xfadelength;
    w->xfadevalue = xfadevalue;
}

// read one sample from delay buffer, with linear interpolation
static inline float read_sample(_tSOLAD* const w, float floatindex)
{
    int index = (int)floatindex;
    float fraction = floatindex - (float)index;
    float *buf = w->delaybuf;
    index &= LOOPMASK;
    
    return (buf[index] + (fraction * (buf[index+1] - buf[index])));
}

static void solad_init(_tSOLAD* const w)
{
    w->timeindex = 0;
    w->xfadevalue = -1;
    w->period = INITPERIOD;
    w->readlag = INITPERIOD;
    w->blocksize = INITPERIOD;
}

//============================================================================================================
// PITCHSHIFT
//============================================================================================================

static int pitchshift_attackdetect(_tPitchShift* ps)
{
    float envout;
    
    _tPeriodDetection* p = *ps->p;
    
    envout = tEnvPD_tick(&p->env);
    
    if (envout >= 1.0f)
    {
        p->lastmax = p->max;
        if (envout > p->max)
        {
            p->max = envout;
        }
        else
        {
            p->deltamax = envout - p->max;
            p->max = p->max * ps->radius;
        }
        p->deltamax = p->max - p->lastmax;
    }
    
    p->fba = p->fba ? (p->fba - 1) : 0;
    
    return (p->fba == 0 && (p->max > 60 && p->deltamax > 6)) ? 1 : 0;
}

void tPitchShift_init (tPitchShift* const psr, tPeriodDetection* pd, float* out, int bufSize)
{
    tPitchShift_initToPool(psr, pd, out, bufSize, &leaf.mempool);
}

void tPitchShift_initToPool (tPitchShift* const psr, tPeriodDetection* const pd, float* out, int bufSize, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tPitchShift* ps = *psr = (_tPitchShift*) mpool_calloc(sizeof(_tPitchShift), m);
    ps->mempool = m;
    
    _tPeriodDetection* p = *pd;
    
    ps->p = pd;
    
    ps->outBuffer = out;
    ps->bufSize = bufSize;
    ps->frameSize = p->frameSize;
    ps->framesPerBuffer = ps->bufSize / ps->frameSize;
    ps->curBlock = 1;
    ps->lastBlock = 0;
    ps->index = 0;
    ps->pitchFactor = 1.0f;
    
    tSOLAD_initToPool(&ps->sola, mp);
    
    tHighpass_initToPool(&ps->hp, HPFREQ, mp);
    
    tSOLAD_setPitchFactor(&ps->sola, DEFPITCHRATIO);
}

void tPitchShift_free (tPitchShift* const psr)
{
    _tPitchShift* ps = *psr;
    
    tSOLAD_free(&ps->sola);
    tHighpass_free(&ps->hp);
    mpool_free((char*)ps, ps->mempool);
}

void tPitchShift_setPitchFactor(tPitchShift* psr, float pf)
{
    _tPitchShift* ps = *psr;
    
    ps->pitchFactor = pf;
}

float tPitchShift_shift (tPitchShift* psr)
{
    _tPitchShift* ps = *psr;
    _tPeriodDetection* p = *ps->p;
    
    float period, out;
    int i, iLast;
    
    i = p->i;
    iLast = p->iLast;
    
    out = tHighpass_tick(&ps->hp, ps->outBuffer[iLast]);
    
    if (p->indexstore >= ps->frameSize)
    {
        period = tPeriodDetection_getPeriod(&p);
        
        if(pitchshift_attackdetect(ps) == 1)
        {
            p->fba = 5;
            tSOLAD_setReadLag(&ps->sola, p->windowSize);
        }
        
        tSOLAD_setPeriod(&ps->sola, period);
        tSOLAD_setPitchFactor(&ps->sola, ps->pitchFactor);
        
        tSOLAD_ioSamples(&ps->sola, &(p->inBuffer[i]), &(ps->outBuffer[i]), ps->frameSize);
    }
    
    return out;
}

float tPitchShift_shiftToFreq (tPitchShift* psr, float freq)
{
    _tPitchShift* ps = *psr;
    _tPeriodDetection* p = *ps->p;
    
    float period, out;
    int i, iLast;
    
    i = p->i;
    iLast = p->iLast;
    
    out = tHighpass_tick(&ps->hp, ps->outBuffer[iLast]);
    
    if (p->indexstore >= ps->frameSize)
    {
        period = tPeriodDetection_getPeriod(&p);
        
        if(pitchshift_attackdetect(ps) == 1)
        {
            p->fba = 5;
            tSOLAD_setReadLag(&ps->sola, p->windowSize);
        }
        
        tSOLAD_setPeriod(&ps->sola, period);
        
        if (period != 0) ps->pitchFactor = period*freq*leaf.invSampleRate;
        else ps->pitchFactor = 1.0f;
        
        tSOLAD_setPitchFactor(&ps->sola, ps->pitchFactor);
        
        tSOLAD_ioSamples(&ps->sola, &(p->inBuffer[i]), &(ps->outBuffer[i]), ps->frameSize);
    }
    return out;
}

float tPitchShift_shiftToFunc (tPitchShift* psr, float (*fun)(float))
{
    _tPitchShift* ps = *psr;
    _tPeriodDetection* p = *ps->p;
    
    float period, out;
    int i, iLast;
    
    i = p->i;
    iLast = p->iLast;
    
    out = tHighpass_tick(&ps->hp, ps->outBuffer[iLast]);
    
    if (p->indexstore >= ps->frameSize)
    {
        period = tPeriodDetection_getPeriod(&p);
        
        if(pitchshift_attackdetect(ps) == 1)
        {
            p->fba = 5;
            tSOLAD_setReadLag(&ps->sola, p->windowSize);
        }
        
        tSOLAD_setPeriod(&ps->sola, period);
        
        ps->pitchFactor = period/fun(period);
        tSOLAD_setPitchFactor(&ps->sola, ps->pitchFactor);
        
        tSOLAD_ioSamples(&ps->sola, &(p->inBuffer[i]), &(ps->outBuffer[i]), ps->frameSize);
        
        ps->curBlock++;
        if (ps->curBlock >= p->framesPerBuffer) ps->curBlock = 0;
        ps->lastBlock++;
        if (ps->lastBlock >= ps->framesPerBuffer) ps->lastBlock = 0;
    }
    
    return out;
}

//============================================================================================================
// RETUNE
//============================================================================================================

void tRetune_init(tRetune* const rt, int numVoices, int bufSize, int frameSize)
{
    tRetune_initToPool(rt, numVoices, bufSize, frameSize, &leaf.mempool);
}

void tRetune_initToPool (tRetune* const rt, int numVoices, int bufSize, int frameSize, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tRetune* r = *rt = (_tRetune*) mpool_alloc(sizeof(_tRetune), m);
    r->mempool = *mp;
    
    r->bufSize = bufSize;
    r->frameSize = frameSize;
    r->numVoices = numVoices;
    
    r->inBuffer = (float*) mpool_calloc(sizeof(float) * r->bufSize, m);
    r->outBuffers = (float**) mpool_calloc(sizeof(float*) * r->numVoices, m);
    
    r->hopSize = DEFHOPSIZE;
    r->windowSize = DEFWINDOWSIZE;
    r->fba = FBA;
    tRetune_setTimeConstant(rt, DEFTIMECONSTANT);
    
    r->inputPeriod = 0.0f;

    r->ps = (tPitchShift*) mpool_calloc(sizeof(tPitchShift) * r->numVoices, m);
    r->pitchFactor = (float*) mpool_calloc(sizeof(float) * r->numVoices, m);
    r->tickOutput = (float*) mpool_calloc(sizeof(float) * r->numVoices, m);
    for (int i = 0; i < r->numVoices; ++i)
    {
        r->outBuffers[i] = (float*) mpool_calloc(sizeof(float) * r->bufSize, m);
    }
    
    tPeriodDetection_initToPool(&r->pd, r->inBuffer, r->outBuffers[0], r->bufSize, r->frameSize, mp);

    for (int i = 0; i < r->numVoices; ++i)
    {
        tPitchShift_initToPool(&r->ps[i], &r->pd, r->outBuffers[i], r->bufSize, mp);
    }
}

void tRetune_free (tRetune* const rt)
{
    _tRetune* r = *rt;
    
    tPeriodDetection_free(&r->pd);
    for (int i = 0; i < r->numVoices; ++i)
    {
        tPitchShift_free(&r->ps[i]);
        mpool_free((char*)r->outBuffers[i], r->mempool);
    }
    mpool_free((char*)r->tickOutput, r->mempool);
    mpool_free((char*)r->pitchFactor, r->mempool);
    mpool_free((char*)r->ps, r->mempool);
    mpool_free((char*)r->inBuffer, r->mempool);
    mpool_free((char*)r->outBuffers, r->mempool);
    mpool_free((char*)r, r->mempool);
}

float* tRetune_tick(tRetune* const rt, float sample)
{
    _tRetune* r = *rt;
    
    r->inputPeriod = tPeriodDetection_tick(&r->pd, sample);
    
    for (int v = 0; v < r->numVoices; ++v)
    {
        r->tickOutput[v] = tPitchShift_shift(&r->ps[v]);
    }
    
    return r->tickOutput;
}

void tRetune_setNumVoices(tRetune* const rt, int numVoices)
{
    _tRetune* r = *rt;
    
    int bufSize = r->bufSize;
    int frameSize = r->frameSize;
    tMempool mempool = r->mempool;
    
    tRetune_free(rt);
    tRetune_initToPool(rt, numVoices, bufSize, frameSize, &mempool);
}

void tRetune_setPitchFactors(tRetune* const rt, float pf)
{
    _tRetune* r = *rt;
    
    for (int i = 0; i < r->numVoices; ++i)
    {
        r->pitchFactor[i] = pf;
        tPitchShift_setPitchFactor(&r->ps[i], r->pitchFactor[i]);
    }
}

void tRetune_setPitchFactor(tRetune* const rt, float pf, int voice)
{
    _tRetune* r = *rt;
    
    r->pitchFactor[voice] = pf;
    tPitchShift_setPitchFactor(&r->ps[voice], r->pitchFactor[voice]);
}

void tRetune_setTimeConstant(tRetune* const rt, float tc)
{
    _tRetune* r = *rt;
    
    r->timeConstant = tc;
    r->radius = expf(-1000.0f * r->hopSize * leaf.invSampleRate / r->timeConstant);
}

void tRetune_setHopSize(tRetune* const rt, int hs)
{
    _tRetune* r = *rt;
    
    r->hopSize = hs;
    tPeriodDetection_setHopSize(&r->pd, r->hopSize);
}

void tRetune_setWindowSize(tRetune* const rt, int ws)
{
    _tRetune* r = *rt;
    
    r->windowSize = ws;
    tPeriodDetection_setWindowSize(&r->pd, r->windowSize);
}

void tRetune_setFidelityThreshold(tRetune* const rt, float threshold)
{
    _tRetune* r = *rt;
    
    tPeriodDetection_setFidelityThreshold(&r->pd, threshold);
}

float tRetune_getInputPeriod(tRetune* const rt)
{
    _tRetune* r = *rt;
    
    return (r->inputPeriod * leaf.invSampleRate);
}

float tRetune_getInputFreq(tRetune* const rt)
{
    _tRetune* r = *rt;
    
    return 1.0f/(r->inputPeriod * leaf.invSampleRate);
}

//============================================================================================================
// AUTOTUNE
//============================================================================================================

void tAutotune_init (tAutotune* const rt, int numVoices, int bufSize, int frameSize)
{
    tAutotune_initToPool(rt, numVoices, bufSize, frameSize, &leaf.mempool);
}

void tAutotune_initToPool (tAutotune* const rt, int numVoices, int bufSize, int frameSize, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tAutotune* r = *rt = (_tAutotune*) mpool_alloc(sizeof(_tAutotune), m);
    r->mempool = *mp;
    
    r->bufSize = bufSize;
    r->frameSize = frameSize;
    r->numVoices = numVoices;
    
    r->inBuffer = (float*) mpool_alloc(sizeof(float) * r->bufSize, m);
    r->outBuffers = (float**) mpool_alloc(sizeof(float*) * r->numVoices, m);
    
    r->hopSize = DEFHOPSIZE;
    r->windowSize = DEFWINDOWSIZE;
    r->fba = FBA;
    tAutotune_setTimeConstant(rt, DEFTIMECONSTANT);
    
    r->ps = (tPitchShift*) mpool_alloc(sizeof(tPitchShift) * r->numVoices, m);
    r->freq = (float*) mpool_alloc(sizeof(float) * r->numVoices, m);
    r->tickOutput = (float*) mpool_alloc(sizeof(float) * r->numVoices, m);
    for (int i = 0; i < r->numVoices; ++i)
    {
        r->outBuffers[i] = (float*) mpool_alloc(sizeof(float) * r->bufSize, m);
    }
    
    tPeriodDetection_initToPool(&r->pd, r->inBuffer, r->outBuffers[0], r->bufSize, r->frameSize, mp);

    for (int i = 0; i < r->numVoices; ++i)
    {
        tPitchShift_initToPool(&r->ps[i], &r->pd, r->outBuffers[i], r->bufSize, mp);
    }
    
    r->inputPeriod = 0.0f;
    r->shiftOn = 0;
}

void tAutotune_free (tAutotune* const rt)
{
    _tAutotune* r = *rt;
    
    tPeriodDetection_free(&r->pd);
    for (int i = 0; i < r->numVoices; ++i)
    {
        tPitchShift_free(&r->ps[i]);
        mpool_free((char*)r->outBuffers[i], r->mempool);
    }
    mpool_free((char*)r->tickOutput, r->mempool);
    mpool_free((char*)r->freq, r->mempool);
    mpool_free((char*)r->ps, r->mempool);
    mpool_free((char*)r->inBuffer, r->mempool);
    mpool_free((char*)r->outBuffers, r->mempool);
    mpool_free((char*)r, r->mempool);
}

float* tAutotune_tick(tAutotune* const rt, float sample)
{
    _tAutotune* r = *rt;
    
    float tempPeriod = tPeriodDetection_tick(&r->pd, sample);
    if (tempPeriod < 1000.0f) //to avoid trying to follow consonants JS
	{
		r->inputPeriod = tempPeriod;
	}

	for (int v = 0; v < r->numVoices; ++v)
	{
		r->tickOutput[v] = tPitchShift_shiftToFreq(&r->ps[v], r->freq[v]);
	}

    return r->tickOutput;
}

void tAutotune_setNumVoices(tAutotune* const rt, int numVoices)
{
    _tAutotune* r = *rt;
    
    int bufSize = r->bufSize;
    int frameSize = r->frameSize;
    tMempool mempool = r->mempool;
    
    tAutotune_free(rt);
    tAutotune_initToPool(rt, numVoices, bufSize, frameSize, &mempool);
}

void tAutotune_setFreqs(tAutotune* const rt, float f)
{
    _tAutotune* r = *rt;
    
    for (int i = 0; i < r->numVoices; ++i)
    {
        r->freq[i] = f;
    }
}

void tAutotune_setFreq(tAutotune* const rt, float f, int voice)
{
    _tAutotune* r = *rt;
    
    r->freq[voice] = f;
}

void tAutotune_setTimeConstant(tAutotune* const rt, float tc)
{
    _tAutotune* r = *rt;
    
    r->timeConstant = tc;
    r->radius = expf(-1000.0f * r->hopSize * leaf.invSampleRate / r->timeConstant);
}

void tAutotune_setHopSize(tAutotune* const rt, int hs)
{
    _tAutotune* r = *rt;
    
    r->hopSize = hs;
    tPeriodDetection_setHopSize(&r->pd, r->hopSize);
}

void tAutotune_setWindowSize(tAutotune* const rt, int ws)
{
    _tAutotune* r = *rt;
    
    r->windowSize = ws;
    tPeriodDetection_setWindowSize(&r->pd, r->windowSize);
}

void tAutotune_setFidelityThreshold(tAutotune* const rt, float threshold)
{
    _tAutotune* r = *rt;

    tPeriodDetection_setFidelityThreshold(&r->pd, threshold);
}

void     tAutotune_setAlpha                (tAutotune* rt, float alpha)
{
    _tAutotune* r = *rt;
    tPeriodDetection_setAlpha(&r->pd, alpha);
}

void     tAutotune_setTolerance            (tAutotune* rt, float tolerance)
{
    _tAutotune* r = *rt;
    tPeriodDetection_setTolerance(&r->pd, tolerance);
}

float tAutotune_getInputPeriod(tAutotune* const rt)
{
    _tAutotune* r = *rt;
    
    return r->inputPeriod;
}

float tAutotune_getInputFreq(tAutotune* const rt)
{
    _tAutotune* r = *rt;
    
    return 1.0f/r->inputPeriod;
}

//============================================================================================================
// FORMANTSHIFTER
//============================================================================================================
// algorithm from Tom Baran's autotalent code.

void tFormantShifter_init (tFormantShifter* const fsr, int order)
{
    tFormantShifter_initToPool(fsr, order, &leaf.mempool);
}

void tFormantShifter_initToPool (tFormantShifter* const fsr, int order, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tFormantShifter* fs = *fsr = (_tFormantShifter*) mpool_alloc(sizeof(_tFormantShifter), m);
    fs->mempool = m;
    
    fs->ford = order;
    fs->fk = (float*) mpool_calloc(sizeof(float) * fs->ford, m);
    fs->fb = (float*) mpool_calloc(sizeof(float) * fs->ford, m);
    fs->fc = (float*) mpool_calloc(sizeof(float) * fs->ford, m);
    fs->frb = (float*) mpool_calloc(sizeof(float) * fs->ford, m);
    fs->frc = (float*) mpool_calloc(sizeof(float) * fs->ford, m);
    fs->fsig = (float*) mpool_calloc(sizeof(float) * fs->ford, m);
    fs->fsmooth = (float*) mpool_calloc(sizeof(float) * fs->ford, m);
    fs->ftvec = (float*) mpool_calloc(sizeof(float) * fs->ford, m);
    
    fs->fbuff = (float*) mpool_calloc(sizeof(float*) * fs->ford, m);

    
    fs->falph = powf(0.001f, 10.0f * leaf.invSampleRate);
    fs->flamb = -(0.8517f*sqrtf(atanf(0.06583f*leaf.sampleRate))-0.1916f);
    fs->fhp = 0.0f;
    fs->flp = 0.0f;
    fs->flpa = powf(0.001f, 10.0f * leaf.invSampleRate);
    fs->fmute = 1.0f;
    fs->fmutealph = powf(0.001f, 1.0f * leaf.invSampleRate);
    fs->cbi = 0;
    fs->intensity = 1.0f;
    fs->invIntensity = 1.0f;
    tHighpass_initToPool(&fs->hp, 20.0f, mp);
    tHighpass_initToPool(&fs->hp2, 20.0f, mp);
    tFeedbackLeveler_initToPool(&fs->fbl1, 0.8f, .005f, 0.125, 1, mp);
    tFeedbackLeveler_initToPool(&fs->fbl2, 0.8f, .005f, 0.125, 1, mp);
}

void tFormantShifter_free (tFormantShifter* const fsr)
{
    _tFormantShifter* fs = *fsr;
    
    mpool_free((char*)fs->fk, fs->mempool);
    mpool_free((char*)fs->fb, fs->mempool);
    mpool_free((char*)fs->fc, fs->mempool);
    mpool_free((char*)fs->frb, fs->mempool);
    mpool_free((char*)fs->frc, fs->mempool);
    mpool_free((char*)fs->fsig, fs->mempool);
    mpool_free((char*)fs->fsmooth, fs->mempool);
    mpool_free((char*)fs->ftvec, fs->mempool);
    mpool_free((char*)fs->fbuff, fs->mempool);
    tHighpass_free(&fs->hp);
    tHighpass_free(&fs->hp2);
    tFeedbackLeveler_free(&fs->fbl1);
    tFeedbackLeveler_free(&fs->fbl2);
    mpool_free((char*)fs, fs->mempool);
}

float tFormantShifter_tick(tFormantShifter* const fsr, float in)
{
    return tFormantShifter_add(fsr, tFormantShifter_remove(fsr, in));
}

float tFormantShifter_remove(tFormantShifter* const fsr, float in)
{
    _tFormantShifter* fs = *fsr;
    in = tFeedbackLeveler_tick(&fs->fbl1, in);
    in = tHighpass_tick(&fs->hp, in * fs->intensity);
    

    float fa, fb, fc, foma, falph, ford, flamb, tf, fk;

    ford = fs->ford;
    falph = fs->falph;
    foma = (1.0f - falph);
    flamb = fs->flamb;
    
    tf = in;
    
    fa = tf - fs->fhp;
    fs->fhp = tf;
    fb = fa;
    for(int i = 0; i < ford; i++)
    {
        fs->fsig[i] = fa*fa*foma + fs->fsig[i]*falph;
        fc = (fb - fs->fc[i])*flamb + fs->fb[i];
        fs->fc[i] = fc;
        fs->fb[i] = fb;
        fk = fa*fc*foma + fs->fk[i]*falph;
        fs->fk[i] = fk;
        tf = fk/(fs->fsig[i] + 0.000001f);
        tf = tf*foma + fs->fsmooth[i]*falph;
        fs->fsmooth[i] = tf;
        fs->fbuff[i] = tf;
        fb = fc - tf*fa;
        fa = fa - tf*fc;
    }

    //return fa * 0.1f;
    return fa;
}

float tFormantShifter_add(tFormantShifter* const fsr, float in)
{
    _tFormantShifter* fs = *fsr;
    
    float fa, fb, fc, ford, flpa, flamb, tf, tf2, f0resp, f1resp, frlamb;
    ford = fs->ford;

    flpa = fs->flpa;
    flamb = fs->flamb;
    tf = fs->shiftFactor * (1.0f+flamb)/(1.0f-flamb);
    frlamb = (tf-1.0f)/(tf+1.0f);
    
    tf2 = in;
    fa = 0.0f;
    fb = fa;
    for (int i=0; i<ford; i++)
    {
        fc = (fb-fs->frc[i])*frlamb + fs->frb[i];
        tf = fs->fbuff[i];
        fb = fc - tf*fa;
        fs->ftvec[i] = tf*fc;
        fa = fa - fs->ftvec[i];
    }
    tf = -fa;
    for (int i=ford-1; i>=0; i--)
    {
        tf = tf + fs->ftvec[i];
    }
    f0resp = tf;
    
    //  second time: compute 1-response
    fa = 1.0f;
    fb = fa;
    for (int i=0; i<ford; i++)
    {
        fc = (fb-fs->frc[i])*frlamb + fs->frb[i];
        tf = fs->fbuff[i];
        fb = fc - tf*fa;
        fs->ftvec[i] = tf*fc;
        fa = fa - fs->ftvec[i];
    }
    tf = -fa;
    for (int i=ford-1; i>=0; i--)
    {
        tf = tf + fs->ftvec[i];
    }
    f1resp = tf;
    
    //  now solve equations for output, based on 0-response and 1-response
    tf = 2.0f*tf2;
    tf2 = tf;
    tf = (1.0f - f1resp + f0resp);
    if (tf!=0.0f)
    {
        tf2 = (tf2 + f0resp) / tf;
    }
    else
    {
        tf2 = 0.0f;
    }
    
    //  third time: update delay registers
    fa = tf2;
    fb = fa;
    for (int i=0; i<ford; i++)
    {
        fc = (fb-fs->frc[i])*frlamb + fs->frb[i];
        fs->frc[i] = fc;
        fs->frb[i] = fb;
        tf = fs->fbuff[i];
        fb = fc - tf*fa;
        fa = fa - tf*fc;
    }
    tf = tf2;
    tf = tf + flpa * fs->flp;  // lowpass post-emphasis filter
    fs->flp = tf;
    
    // Bring up the gain slowly when formant correction goes from disabled
    // to enabled, while things stabilize.
    if (fs->fmute>0.5f)
    {
        tf = tf*(fs->fmute - 0.5f)*2.0f;
    }
    else
    {
        tf = 0.0f;
    }
    tf2 = fs->fmutealph;
    fs->fmute = (1.0f-tf2) + tf2*fs->fmute;
    // now tf is signal output
    // ...and we're done messing with formants
    //tf = tFeedbackLeveler_tick(&fs->fbl2, tf);
    tf = tHighpass_tick(&fs->hp2, tanhf(tf));

    return tf * fs->invIntensity;
}

// 1.0f is no change, 2.0f is an octave up, 0.5f is an octave down
void tFormantShifter_setShiftFactor(tFormantShifter* const fsr, float shiftFactor)
{
    _tFormantShifter* fs = *fsr;
    fs->shiftFactor = shiftFactor;
}

void tFormantShifter_setIntensity(tFormantShifter* const fsr, float intensity)
{
    _tFormantShifter* fs = *fsr;



    fs->intensity = LEAF_clip(1.0f, intensity, 100.0f);

   // tFeedbackLeveler_setTargetLevel(&fs->fbl1, fs->intensity);
    //tFeedbackLeveler_setTargetLevel(&fs->fbl2, fs->intensity);
    //make sure you don't divide by zero, doofies
    if (fs->intensity != 0.0f)
    {
    	fs->invIntensity = 1.0f/fs->intensity;
    }
    else
    {
    	fs->invIntensity = 1.0f;
    }

}
