/*==============================================================================
 leaf-oscillators.h
 Created: 20 Jan 2017 12:00:58pm
 Author:  Michael R Mulshine
 ==============================================================================*/

#ifndef LEAF_OSCILLATORS_H_INCLUDED
#define LEAF_OSCILLATORS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
    
    //==============================================================================
    
#include "leaf-math.h"
#include "leaf-mempool.h"
#include "leaf-tables.h"
#include "leaf-filters.h"
    
    
    /*!
     Header.
     @include basic-oscillators.h
     @example basic-oscillators.c
     An example.
     */

    
    //==============================================================================
    
    /*!
     @defgroup tcycle tCycle
     @ingroup oscillators
     @brief A cycle/sine waveform oscillator. Uses wavetable synthesis.
     @{
     */
    
    /*!
     @fn void    tCycle_init         (tCycle* const osc)
     @brief Initialize a tCycle to the default LEAF mempool.
     @param osc A pointer to the tCycle to be initialized.
     */
    
    /*!
     @fn void    tCycle_initToPool   (tCycle* const osc, tMempool* const pool)
     @brief Initialize a tCycle to a specified mempool.
     @param osc A pointer to the tCycle to be initialized.
     @param pool A pointer to the tMempool to which the tCycle should be initialized.
     */
    
    /*!
     @fn void    tCycle_free         (tCycle* const osc)
     @brief Free a tCycle from the default LEAF mempool.
     @param osc A pointer to the tCycle to be freed.
     */
    
    /*!
     @fn float   tCycle_tick         (tCycle* const osc)
     @brief Tick a tCycle oscillator.
     @param osc A pointer to the relevant tCycle.
     @return The ticked sample as a float from -1 to 1.
     */
    
    /*!
     @fn void    tCycle_setFreq      (tCycle* const osc, float freq)
     @brief Set the frequency of a tCycle oscillator.
     @param osc A pointer to the relevant tCycle.
     @param freq The frequency to set the oscillator to.
     */
    
    /*! @} */
    
    typedef struct _tCycle
    {
        tMempool mempool;
        // Underlying phasor
        float phase;
        float inc,freq;
    } _tCycle;
    
    typedef _tCycle* tCycle;
    
    void    tCycle_init         (tCycle* const osc);
    void    tCycle_initToPool   (tCycle* const osc, tMempool* const pool);
    void    tCycle_free         (tCycle* const osc);
    
    float   tCycle_tick         (tCycle* const osc);
    void    tCycle_setFreq      (tCycle* const osc, float freq);
    
    //==============================================================================
    
    /*!
     @defgroup ttriangle tTriangle
     @ingroup oscillators
     @brief Anti-aliased Triangle waveform using wavetable interpolation. Wavetables constructed from sine components.
     @{
     */
    
    /*!
     @fn void    tTriangle_init         (tTriangle* const osc)
     @brief Initialize a tTriangle to the default LEAF mempool.
     @param osc A pointer to the tTriangle to be initialized.
     */
    
    /*!
     @fn void    tTriangle_initToPool   (tTriangle* const osc, tMempool* const pool)
     @brief Initialize a tTriangle to a specified mempool.
     @param osc A pointer to the tTriangle to be initialized.
     @param pool A pointer to the tMempool to which the tTriangle should be initialized.
     */
    
    /*!
     @fn void    tTriangle_free         (tTriangle* const osc)
     @brief Free a tTriangle from the default LEAF mempool.
     @param osc A pointer to the tTriangle to be freed.
     */
    
    /*!
     @fn float   tTriangle_tick         (tTriangle* const osc)
     @brief Tick a tTriangle oscillator.
     @param osc A pointer to the relevant tTriangle.
     @return The ticked sample as a float from -1 to 1.
     */
    
    /*!
     @fn void    tTriangle_setFreq      (tTriangle* const osc, float freq)
     @brief Set the frequency of a tTriangle oscillator.
     @param osc A pointer to the relevant tTriangle.
     @param freq The frequency to set the oscillator to.
     */
    
    /*! @} */

    typedef struct _tTriangle
    {
        tMempool mempool;
        // Underlying phasor
        float phase;
        float inc,freq;
        int oct;
        float w;
    } _tTriangle;
    
    typedef _tTriangle* tTriangle;

    void    tTriangle_init          (tTriangle* const osc);
    void    tTriangle_initToPool    (tTriangle* const osc, tMempool* const pool);
    void    tTriangle_free          (tTriangle* const osc);
    
    float   tTriangle_tick          (tTriangle* const osc);
    void    tTriangle_setFreq       (tTriangle* const osc, float freq);

    
    //==============================================================================
    
    /*!
     @defgroup tsquare tSquare
     @ingroup oscillators
     @brief Anti-aliased Square waveform using wavetable interpolation. Wavetables constructed from sine components.
     @{
     */
    
    /*!
     @fn void    tSquare_init         (tSquare* const osc)
     @brief Initialize a tSquare to the default LEAF mempool.
     @param osc A pointer to the tSquare to be initialized.
     */
    
    /*!
     @fn void    tSquare_initToPool   (tSquare* const osc, tMempool* const pool)
     @brief Initialize a tSquare to a specified mempool.
     @param osc A pointer to the tSquare to be initialized.
     @param pool A pointer to the tMempool to which the tSquare should be initialized.
     */
    
    /*!
     @fn void    tSquare_free         (tSquare* const osc)
     @brief Free a tSquare from the default LEAF mempool.
     @param osc A pointer to the tSquare to be freed.
     */
    
    /*!
     @fn float   tSquare_tick         (tSquare* const osc)
     @brief Tick a tSquare oscillator.
     @param osc A pointer to the relevant tSquare.
     @return The ticked sample as a float from -1 to 1.
     */
    
    /*!
     @fn void    tSquare_setFreq      (tSquare* const osc, float freq)
     @brief Set the frequency of a tSquare oscillator.
     @param osc A pointer to the relevant tSquare.
     @param freq The frequency to set the oscillator to.
     */
    
    /*! @} */
    
    typedef struct _tSquare
    {
        tMempool mempool;
        // Underlying phasor
        float phase;
        float inc,freq;
        int oct;
        float w;
    } _tSquare;
    
    typedef _tSquare* tSquare;

    void    tSquare_init        (tSquare* const osc);
    void    tSquare_initToPool  (tSquare* const osc, tMempool* const);
    void    tSquare_free        (tSquare* const osc);

    float   tSquare_tick        (tSquare* const osc);
    void    tSquare_setFreq     (tSquare* const osc, float freq);
    
    /*! @} */
    
    //==============================================================================
    
    /*!
     @defgroup tsawtooth tSawtooth
     @ingroup oscillators
     @brief Anti-aliased Square waveform using wavetable interpolation. Wavetables constructed from sine components.
     @{
     */
    
    /*!
     @fn void    tSawtooth_init         (tSawtooth* const osc)
     @brief Initialize a tSawtooth to the default LEAF mempool.
     @param osc A pointer to the tSawtooth to be initialized.
     */
    
    /*!
     @fn void    tSawtooth_initToPool   (tSawtooth* const osc, tMempool* const pool)
     @brief Initialize a tSawtooth to a specified mempool.
     @param osc A pointer to the tSawtooth to be initialized.
     @param pool A pointer to the tMempool to which the tSawtooth should be initialized.
     */
    
    /*!
     @fn void    tSawtooth_free         (tSawtooth* const osc)
     @brief Free a tSawtooth from the default LEAF mempool.
     @param osc A pointer to the tSawtooth to be freed.
     */
    
    /*!
     @fn float   tSawtooth_tick         (tSawtooth* const osc)
     @brief Tick a tSawtooth oscillator.
     @param osc A pointer to the relevant tSawtooth.
     @return The ticked sample as a float from -1 to 1.
     */
    
    /*!
     @fn void    tSawtooth_setFreq      (tSawtooth* const osc, float freq)
     @brief Set the frequency of a tSawtooth oscillator.
     @param osc A pointer to the relevant tSawtooth.
     @param freq The frequency to set the oscillator to.
     */
    
    /*! @} */
    
    typedef struct _tSawtooth
    {
        tMempool mempool;
        // Underlying phasor
        float phase;
        float inc,freq;
        int oct;
        float w;
    } _tSawtooth;
    
    typedef _tSawtooth* tSawtooth;

    void    tSawtooth_init          (tSawtooth* const osc);
    void    tSawtooth_initToPool    (tSawtooth* const osc, tMempool* const pool);
    void    tSawtooth_free          (tSawtooth* const osc);

    float   tSawtooth_tick          (tSawtooth* const osc);
    void    tSawtooth_setFreq       (tSawtooth* const osc, float freq);
    
    //==============================================================================
    
    
    // TO BE REMOVED
    typedef struct _tSine
    {
        tMempool mempool;
        float* sine;
        int size;
        float phase;
        float inc,freq;
    } _tSine;
    
    typedef _tSine* tSine;
    
    void    tSine_init         (tSine* const osc, int size);
    void    tSine_initToPool   (tSine* const osc, int size, tMempool* const pool);

    void    tSine_free         (tSine* const osc);
    float   tSine_tick         (tSine* const osc);
    void    tSine_setFreq      (tSine* const osc, float freq);
    
    //==============================================================================
    
    /*!
     @defgroup ttri tTri
     @ingroup oscillators
     @brief
     @{
     */
    
    /*!
     @fn void    tTri_init          (tTri* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void    tTri_initToPool    (tTri* const osc, tMempool* const pool)
     @brief
     @param
     */
    
    /*!
     @fn void    tTri_free          (tTri* const osc)
     @brief
     @param
     */
    
    /*!
     @fn float   tTri_tick          (tTri* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void    tTri_setFreq       (tTri* const osc, float freq)
     @brief
     @param
     */
    
    /*!
     @fn void    tTri_setSkew       (tTri* const osc, float skew)
     @brief
     @param
     */
    
    /*! @} */

    typedef struct _tTri
    {
        tMempool mempool;
        float phase;
        float inc,freq;
        float skew;
        float lastOut;
    } _tTri;
    
    typedef _tTri* tTri;
    
    void    tTri_init          (tTri* const osc);
    void    tTri_initToPool    (tTri* const osc, tMempool* const pool);
    void    tTri_free          (tTri* const osc);
    
    float   tTri_tick          (tTri* const osc);
    void    tTri_setFreq       (tTri* const osc, float freq);
    void    tTri_setSkew       (tTri* const osc, float skew);
    
    //==============================================================================
    
    /*!
     @defgroup tpulse tPulse
     @ingroup oscillators
     @brief
     @{
     */
    
    /*!
     @fn void    tPulse_init        (tPulse* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void    tPulse_initToPool  (tPulse* const osc, tMempool* const)
     @brief
     @param
     */
    
    /*!
     @fn void    tPulse_free        (tPulse* const osc)
     @brief
     @param
     */
    
    /*!
     @fn float   tPulse_tick        (tPulse* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void    tPulse_setFreq     (tPulse* const osc, float freq)
     @brief
     @param
     */
    
    /*!
     @fn void    tPulse_setWidth    (tPulse* const osc, float width)
     @brief
     @param
     */
    
    /*! @} */
    
    typedef struct _tPulse
    {
        tMempool mempool;
        float phase;
        float inc,freq;
        float width;
    } _tPulse;
    
    typedef _tPulse* tPulse;
    
    void    tPulse_init        (tPulse* const osc);
    void    tPulse_initToPool  (tPulse* const osc, tMempool* const);
    void    tPulse_free        (tPulse* const osc);
    
    float   tPulse_tick        (tPulse* const osc);
    void    tPulse_setFreq     (tPulse* const osc, float freq);
    void    tPulse_setWidth    (tPulse* const osc, float width);
    
    //==============================================================================
    
    /*!
     @defgroup tsaw tSaw
     @ingroup oscillators
     @brief
     @{
     */
    
    /*!
     @fn void    tSaw_init          (tSaw* const osc)
     @brief
     @param
     */
     
    /*!
     @fn void    tSaw_initToPool    (tSaw* const osc, tMempool* const pool)
     @brief
     @param
     */
    
    /*!
     @fn void    tSaw_free          (tSaw* const osc)
     @brief
     @param
     */
    
    /*!
     @fn float   tSaw_tick          (tSaw* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void    tSaw_setFreq       (tSaw* const osc, float freq)
     @brief
     @param
     */
    
    /*! @} */
    
    typedef struct _tSaw
    {
        tMempool mempool;
        float phase;
        float inc,freq;
    } _tSaw;
    
    typedef _tSaw* tSaw;
    
    void    tSaw_init          (tSaw* const osc);
    void    tSaw_initToPool    (tSaw* const osc, tMempool* const pool);
    void    tSaw_free          (tSaw* const osc);
    
    float   tSaw_tick          (tSaw* const osc);
    void    tSaw_setFreq       (tSaw* const osc, float freq);
    
    //==============================================================================
    
    /*!
     @defgroup tphasor tPhasor
     @ingroup oscillators
     @brief
     @{
     */
    
    /*!
     @fn void    tPhasor_init        (tPhasor* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void    tPhasor_initToPool  (tPhasor* const osc, tMempool* const)
     @brief
     @param
     */
    
    /*!
     @fn void    tPhasor_free        (tPhasor* const osc)
     @brief
     @param
     */
    
    /*!
     @fn float   tPhasor_tick        (tPhasor* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void    tPhasor_setFreq     (tPhasor* const osc, float freq)
     @brief
     @param
     */
    
    /*! @} */
    
    typedef struct _tPhasor
    {
        tMempool mempool;
        float phase;
        float inc,freq;
        uint8_t phaseDidReset;
    } _tPhasor;
    
    typedef _tPhasor* tPhasor;
    
    void    tPhasor_init        (tPhasor* const osc);
    void    tPhasor_initToPool  (tPhasor* const osc, tMempool* const);
    void    tPhasor_free        (tPhasor* const osc);
    
    float   tPhasor_tick        (tPhasor* const osc);
    void    tPhasor_setFreq     (tPhasor* const osc, float freq);
    
    //==============================================================================
    
    /*!
     @defgroup tnoise tNoise
     @ingroup oscillators
     @brief
     @{
     */
    
    /*!
     @fn void    tNoise_init         (tNoise* const noise, NoiseType type)
     @brief
     @param
     */
    
    /*!
     @fn void    tNoise_initToPool   (tNoise* const noise, NoiseType type, tMempool* const)
     @brief
     @param
     */
    
    /*!
     @fn void    tNoise_free         (tNoise* const noise)
     @brief
     @param
     */
    
    /*!
     @fn float   tNoise_tick         (tNoise* const noise)
     @brief
     @param
     */
    
    /* tNoise. WhiteNoise, PinkNoise. */
    /*!
     * Noise types
     */
    enum NoiseType
    {
        WhiteNoise, //!< White noise. Full spectrum.
        PinkNoise, //!< Pink noise. Inverse frequency-proportional spectrum.
        NoiseTypeNil,
    };
    
     /*! @} */
    
    typedef enum NoiseType NoiseType;
    
    typedef struct _tNoise
    {
        tMempool mempool;
        NoiseType type;
        float pinkb0, pinkb1, pinkb2;
        float(*rand)(void);
    } _tNoise;
    
    typedef _tNoise* tNoise;

    void    tNoise_init         (tNoise* const noise, NoiseType type);
    void    tNoise_initToPool   (tNoise* const noise, NoiseType type, tMempool* const);
    void    tNoise_free         (tNoise* const noise);
    
    float   tNoise_tick         (tNoise* const noise);
    
    //==============================================================================
    
    /*!
     @defgroup tneuron tNeuron
     @ingroup oscillators
     @brief A model of a neuron, adapted to act as an oscillator.
     @{
     */
    
    /*!
     @fn void    tNeuron_init        (tNeuron* const neuron)
     @brief Initialize a tNeuron to the default LEAF mempool.
     @param neuron A pointer to the tNeuron to be initialized.
     */
    
    /*!
     @fn void    tNeuron_initToPool  (tNeuron* const neuron, tMempool* const pool)
     @brief Initialize a tNeuron to a specified mempool.
     @param neuron A pointer to the tNeuron to be initialized.
     @param pool A pointer to the tMempool to which the tNeuron should be initialized.
     */
    
    /*!
     @fn void    tNeuron_free        (tNeuron* const neuron)
     @brief Free a tNeuron from the default LEAF mempool.
     @param neuron A pointer to the tNeuron to be freed.
     */
    
    /*!
     @fn void    tNeuron_reset       (tNeuron* const neuron)
     @brief Reset the neuron model.
     @param neuron A pointer to the relevant tNeuron.
     */
    
    /*!
     @fn float   tNeuron_tick        (tNeuron* const neuron)
     @brief Tick a tNeuron oscillator.
     @param neuron A pointer to the relevant tNeuron.
     @return The ticked sample as a float from -1 to 1.
     */

    /*!
     @fn void    tNeuron_setMode     (tNeuron* const neuron, NeuronMode mode)
     @brief Set the tNeuron shaping mode.
     @param neuron A pointer to the relevant tNeuron.
     @param mode The mode to set the tNeuron to.
     */

    /*!
     @fn void    tNeuron_setCurrent  (tNeuron* const neuron, float current)
     @brief Set the current.
     @param neuron A pointer to the relevant tNeuron.
     @param current The new current.
     */
    
    /*!
     @fn void    tNeuron_setK        (tNeuron* const neuron, float K)
     @brief Set the potassium value.
     @param neuron A pointer to the relevant tNeuron.
     @param K The new potassium.
     */
    
    /*!
     @fn void    tNeuron_setL        (tNeuron* const neuron, float L)
     @brief Set the chloride value.
     @param neuron A pointer to the relevant tNeuron.
     @param L The new chloride value.
     */
    
    /*!
     @fn void    tNeuron_setN        (tNeuron* const neuron, float N)
     @brief Set the sodium value.
     @param neuron A pointer to the relevant tNeuron.
     @param N The new sodium value.
     */
    
    /*!
     @fn void    tNeuron_setC        (tNeuron* const neuron, float C)
     @brief Set the calcium value.
     @param neuron A pointer to the relevant tNeuron.
     @param C The new calcium.
     */
    
    /*!
     @fn  void    tNeuron_setV1       (tNeuron* const neuron, float V1)
     @brief Set the V1 value.
     @param neuron A pointer to the relevant tNeuron.
     @param V1 The new V1.
     */
    
    /*!
     @fn void    tNeuron_setV2       (tNeuron* const neuron, float V2)
     @brief Set the V2 value.
     @param neuron A pointer to the relevant tNeuron.
     @param V2 The new V2.
     */
    
    /*!
     @fn void    tNeuron_setV3       (tNeuron* const neuron, float V3)
     @brief Set the V3 value.
     @param neuron A pointer to the relevant tNeuron.
     @param V3 The new V3.
     */
    
    /*!
     @fn void    tNeuron_setTimeStep (tNeuron* const neuron, float timestep)
     @brief Set the time step of the model.
     @param neuron A pointer to the relevant tNeuron.
     @param timestep The new time step.
     */
    
    /*!
     @brief Shaping modes for tNeuron output.
     */
    enum NeuronMode
    {
        NeuronNormal, //!< Normal operation
        NeuronTanh, //!< Tanh voltage shaping
        NeuronAaltoShaper, //!< Aalto voltage shaping
        NeuronModeNil
    };
    
    /*! @} */
    
    typedef enum NeuronMode NeuronMode;
    
    typedef struct _tNeuron
    {
        tMempool mempool;
        
        tPoleZero f;
        
        NeuronMode mode;
        
        float voltage, current;
        float timeStep;
        
        float alpha[3];
        float beta[3];
        float rate[3];
        float V[3];
        float P[3];
        float gK, gN, gL, C;
    } _tNeuron;
    
    typedef _tNeuron* tNeuron;
    
    void    tNeuron_init        (tNeuron* const neuron);
    void    tNeuron_initToPool  (tNeuron* const neuron, tMempool* const pool);
    void    tNeuron_free        (tNeuron* const neuron);
    
    void    tNeuron_reset       (tNeuron* const neuron);
    float   tNeuron_tick        (tNeuron* const neuron);
    void    tNeuron_setMode     (tNeuron* const neuron, NeuronMode mode);
    void    tNeuron_setCurrent  (tNeuron* const neuron, float current);
    void    tNeuron_setK        (tNeuron* const neuron, float K);
    void    tNeuron_setL        (tNeuron* const neuron, float L);
    void    tNeuron_setN        (tNeuron* const neuron, float N);
    void    tNeuron_setC        (tNeuron* const neuron, float C);
    void    tNeuron_setV1       (tNeuron* const neuron, float V1);
    void    tNeuron_setV2       (tNeuron* const neuron, float V2);
    void    tNeuron_setV3       (tNeuron* const neuron, float V3);
    void    tNeuron_setTimeStep (tNeuron* const neuron, float timestep);
    

    
    //==============================================================================
    
    
#define FILLEN 256
    
    /*!
     @defgroup tmbpulse tMBPulse
     @ingroup oscillators
     @brief
     @{
     */
    
    /*!
     @fn void tMBPulse_init(tMBPulse* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void tMBPulse_initToPool(tMBPulse* const osc, tMempool* const pool)
     @brief
     @param
     */
    
    /*!
     @fn void tMBPulse_free(tMBPulse* const osc)
     @brief
     @param
     */
    
    /*!
     @fn float tMBPulse_tick(tMBPulse* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void tMBPulse_setFreq(tMBPulse* const osc, float f)
     @brief
     @param
     */
    
    /*!
     @fn void tMBPulse_setWidth(tMBPulse* const osc, float w)
     @brief
     @param
     */
    
    /*!
     @fn void tMBPulse_syncIn(tMBPulse* const osc, float sync)
     @brief
     @param
     */
    
    /*!
     @fn float tMBPulse_syncOut(tMBPulse* const osc)
     @brief
     @param
     */
    
    /*! @} */
    
    typedef struct _tMBPulse
    {
        tMempool mempool;
        float    out;
        float    amp;
        float    last_amp;
        float    freq;
        float    waveform;    // duty cycle, must be in [-1, 1]
        float    syncin;
        float    syncout;
        float   _p, _w, _b, _x, _z;
        float   _f [FILLEN + STEP_DD_PULSE_LENGTH];
        int     _j, _k;
        bool    _init;
    } _tMBPulse;
    
    typedef _tMBPulse* tMBPulse;
    
    void tMBPulse_init(tMBPulse* const osc);
    void tMBPulse_initToPool(tMBPulse* const osc, tMempool* const pool);
    void tMBPulse_free(tMBPulse* const osc);
    
    float tMBPulse_tick(tMBPulse* const osc);
    void tMBPulse_setFreq(tMBPulse* const osc, float f);
    void tMBPulse_setWidth(tMBPulse* const osc, float w);
    void tMBPulse_syncIn(tMBPulse* const osc, float sync);
    float tMBPulse_syncOut(tMBPulse* const osc);
    
    /*!
     @defgroup tmbtriangle tMBTriangle
     @ingroup oscillators
     @brief
     @{
     */
    
    /*!
     @fn void tMBTriangle_init(tMBTriangle* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void tMBTriangle_initToPool(tMBTriangle* const osc, tMempool* const pool)
     @brief
     @param
     */
    
    /*!
     @fn void tMBTriangle_free(tMBTriangle* const osc)
     @brief
     @param
     */
    
    /*!
     @fn float tMBTriangle_tick(tMBTriangle* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void tMBTriangle_setFreq(tMBTriangle* const osc, float f)
     @brief
     @param
     */
    
    /*!
     @fn void tMBTriangle_setWidth(tMBTriangle* const osc, float w)
     @brief
     @param
     */
    
    /*!
     @fn void tMBTriangle_syncIn(tMBTriangle* const osc, float sync)
     @brief
     @param
     */
    
    /*!
     @fn float tMBTriangle_syncOut(tMBTriangle* const osc)
     @brief
     @param
     */
    
    /*! @} */
    
    typedef struct _tMBTriangle
    {
        tMempool mempool;
        float    out;
        float    amp;
        float    last_amp;
        float    freq;
        float    waveform;    // duty cycle, must be in [-1, 1]
        float    syncin;
        float    syncout;
        float   _p, _w, _b, _z;
        float   _f [FILLEN + LONGEST_DD_PULSE_LENGTH];
        int     _j, _k;
        bool    _init;
    } _tMBTriangle;
    
    typedef _tMBTriangle* tMBTriangle;
    
    void tMBTriangle_init(tMBTriangle* const osc);
    void tMBTriangle_initToPool(tMBTriangle* const osc, tMempool* const pool);
    void tMBTriangle_free(tMBTriangle* const osc);
    
    float tMBTriangle_tick(tMBTriangle* const osc);
    void tMBTriangle_setFreq(tMBTriangle* const osc, float f);
    void tMBTriangle_setWidth(tMBTriangle* const osc, float w);
    void tMBTriangle_syncIn(tMBTriangle* const osc, float sync);
    float tMBTriangle_syncOut(tMBTriangle* const osc);
    
    
    /*!
     @defgroup tmbsaw tMBSaw
     @ingroup oscillators
     @brief
     @{
     */

    /*!
     @fn void tMBSaw_init(tMBSaw* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void tMBSaw_initToPool(tMBSaw* const osc, tMempool* const pool)
     @brief
     @param
     */
    
    /*!
     @fn void tMBSaw_free(tMBSaw* const osc)
     @brief
     @param
     */
    
    /*!
     @fn float tMBSaw_tick(tMBSaw* const osc)
     @brief
     @param
     */
    
    /*!
     @fn void tMBSaw_setFreq(tMBSaw* const osc, float f)
     @brief
     @param
     */
    
    /*!
     @fn void tMBSaw_syncIn(tMBSaw* const osc, float sync)
     @brief
     @param
     */
    
    /*!
     @fn float tMBSaw_syncOut(tMBSaw* const osc)
     @brief
     @param
     */
    
    /*! @} */
    
    typedef struct _tMBSaw
    {
        tMempool mempool;
        float    out;
        float    amp;
        float    last_amp;
        float    freq;
        float    syncin;
        float    syncout;
        float   _p, _w, _z;
        float   _f [FILLEN + STEP_DD_PULSE_LENGTH];
        int     _j;
        bool    _init;
    } _tMBSaw;
    
    typedef _tMBSaw* tMBSaw;
    
    void tMBSaw_init(tMBSaw* const osc);
    void tMBSaw_initToPool(tMBSaw* const osc, tMempool* const pool);
    void tMBSaw_free(tMBSaw* const osc);
    
    float tMBSaw_tick(tMBSaw* const osc);
    void tMBSaw_setFreq(tMBSaw* const osc, float f);
    void tMBSaw_syncIn(tMBSaw* const osc, float sync);
    float tMBSaw_syncOut(tMBSaw* const osc);
    
    
    
    
#ifdef __cplusplus
}
#endif

#endif  // LEAF_OSCILLATORS_H_INCLUDED

//==============================================================================


