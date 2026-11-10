// =============================================================================
//  METEORA DRUM - motore audio (C++ puro, nessuna dipendenza da JUCE)
//  Meteora Plugins
// =============================================================================
#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <atomic>

namespace md
{
constexpr float kPi = 3.14159265358979f;

inline float dbToGain (float db) { return db <= -99.0f ? 0.0f : std::pow (10.0f, db * 0.05f); }

// -----------------------------------------------------------------------------
//  Pezzi della batteria (= canali del mixer) e campioni
// -----------------------------------------------------------------------------
enum Piece { Kick = 0, Snare, Tom, Floor, HiHat, CrashL, CrashR, Ride, NumPieces };
// indici dei meter: i pezzi (0-7), poi i bus e il master
enum Meter { MeterOH = NumPieces, MeterRoom, MeterMaster, NumMeters };

// i crash non hanno canale: si sentono solo negli overheads e nella room
inline bool hasCloseChannel (int piece) { return piece != CrashL && piece != CrashR; }

enum Slot
{
    sKick = 0, sSnare, sTom, sFloor,
    sHH1, sHH2, sHH3, sHH4, sHH5, sHH6, sHH7,   // file hh_pos_1 ... hh_pos_7
    sHHSplash, sHHChick,
    sCrashL, sCrashR, sRide,
    // set B (registrazioni nuove)
    sKickB, sSnareB, sRimshot, sTomB, sFloorB,
    sHHB1, sHHB2, sHHB3, sHHB4, sHHBPedal,       // B: 1 = chiuso ... 4 = aperto
    sCrashLB, sCrashRB,
    NumSlots
};

// gruppi con scelta A/B
enum SetGroup { gKick = 0, gSnare, gTom, gFloor, gHiHat, gCrashL, gCrashR, NumSetGroups };

// Ordine di apertura dell'hi-hat (0 = tutto aperto, 6 = tutto chiuso)
// ricavato dall'analisi dei file: 1, 2, 3, 5, 4, 7, 6
constexpr int kHHOpenness[7] = { sHH1, sHH2, sHH3, sHH5, sHH4, sHH7, sHH6 };
// set B: 4 posizioni (0 = aperto, 3 = chiuso)
constexpr int kHHOpennessB[4] = { sHHB4, sHHB3, sHHB2, sHHB1 };

// Nome del file, pezzo e guadagno base (bilanciamento naturale a velocity piena)
struct SlotInfo { const char* resource; int piece; float baseGainDb; };
constexpr SlotInfo kSlotInfo[NumSlots] =
{
    { "kick_wav",      Kick,   -7.3f },
    { "snare_wav",     Snare,   -4.5f },
    { "tom_wav",       Tom,    -11.2f },
    { "floor_wav",     Floor,  -13.2f },
    { "hh_pos1_wav",   HiHat,  -14.9f },
    { "hh_pos2_wav",   HiHat,  -10.1f },
    { "hh_pos3_wav",   HiHat,  -16.1f },
    { "hh_pos4_wav",   HiHat,  -12.3f },
    { "hh_pos5_wav",   HiHat,  -7.5f },
    { "hh_pos6_wav",   HiHat,  -7.0f },
    { "hh_pos7_wav",   HiHat,  -13.3f },
    { "hh_splash_wav", HiHat, -20.5f },
    { "hh_chick_wav",  HiHat,  -9.8f },
    { "crash_l_wav",   CrashL, -11.1f },
    { "crash_r_wav",   CrashR, -7.6f },
    { "ride_wav",      Ride,   -10.7f },
    // set B: guadagni calcolati per avere la stessa loudness percepita (LUFS) del set A
    { "b_kick_wav",     Kick,   -9.6f },
    { "b_snare_wav",    Snare,  -7.2f },
    { "b_rimshot_wav",  Snare,  -4.8f },
    { "b_tom_wav",      Tom,   -12.9f },
    { "b_floor_wav",    Floor, -11.9f },
    { "b_hh_pos1_wav",  HiHat, -14.3f },
    { "b_hh_pos2_wav",  HiHat,  -12.7f },
    { "b_hh_pos3_wav",  HiHat, -13.9f },
    { "b_hh_pos4_wav",  HiHat, -15.2f },
    { "b_hh_pedal_wav", HiHat,  -11.8f },
    { "b_crash_l_wav",  CrashL, -9.7f },
    { "b_crash_r_wav",  CrashR,-11.0f },
};

struct Sample
{
    std::vector<float> data;
    double sampleRate = 48000.0;
    float baseGainDb  = 0.0f;     // bilanciamento naturale a velocity piena
    int   piece       = 0;
};

// Comportamento di ogni pezzo rispetto alla velocity
struct PieceTraits
{
    float brightMinHz;     // cutoff del filtro con colpo leggerissimo
    float attackMaxMs;     // ammorbidimento massimo dell'attacco
    float velPitchCents;   // leggero aumento di intonazione nei colpi forti
    float ohCapture;       // quanto il pezzo arriva negli overheads
    float roomCapture;     // quanto il pezzo arriva nella room
    float kitPos;          // posizione stereo nel kit (-1 sx, +1 dx), vista della foto
    float ohDelayMs;       // distanza dagli overheads
};

constexpr PieceTraits kTraits[NumPieces] =
{
    //  bright  attack  pitch  OH     room   pos     OHdelay
    {  700.f,  3.0f,   6.f,  0.30f, 0.85f,  0.00f, 3.2f },   // Kick
    { 1600.f,  3.5f,   8.f,  0.60f, 1.00f, -0.18f, 2.4f },   // Snare
    { 1000.f,  3.0f,  12.f,  0.55f, 0.90f, -0.22f, 2.0f },   // Tom
    {  900.f,  3.0f,  12.f,  0.50f, 0.90f,  0.55f, 2.6f },   // Floor
    { 2600.f,  2.0f,   0.f,  0.55f, 0.45f, -0.62f, 1.9f },   // HiHat
    { 2200.f,  5.0f,   0.f,  1.00f, 0.60f, -0.55f, 1.2f },   // CrashL
    { 2200.f,  5.0f,   0.f,  1.00f, 0.60f,  0.25f, 1.2f },   // CrashR (piatto centrale)
    { 2600.f,  4.0f,   0.f,  0.65f, 0.40f,  0.60f, 1.4f },   // Ride (piatto grande a destra)
};

// Allineamento di fase: quanto ritardare ogni close rispetto all'arrivo medio agli overhead.
// Valori scelti misurando le cancellazioni tra 50 e 400 Hz (somma reale contro somma di potenze)
// per tutte le distanze e per entrambi i tipi di overhead: è la frazione che dà meno buchi nei bassi.
constexpr float kAlignFrac[NumPieces] = { 1.0f, 0.25f, 0.5f, 0.75f, 0.5f, 0.f, 0.f, 0.5f };

// -----------------------------------------------------------------------------
//  Parametri (scritti dal processore a ogni blocco)
// -----------------------------------------------------------------------------
struct ChannelParams
{
    float levelDb = 0.f, pan = 0.f, tune = 0.f, decay = 1.f, comp = 0.f;
    float ohSend = 1.f, roomSend = 1.f, bleed = 0.f;
    bool  mute = false, solo = false;
};

struct BusParams
{
    float levelDb = 0.f;
    bool  mute = false, solo = false;
    float p1 = 0.f, p2 = 0.f, p3 = 0.f, p4 = 0.f;   // OH: width, locut, air | Room: size, decay, comp, locut
};

struct EngineParams
{
    ChannelParams ch[NumPieces];
    BusParams oh   { -3.f, false, false, 1.0f, 120.f, 2.0f, 0.f };
    BusParams room { -6.f, false, false, 0.5f, 0.9f, 0.5f, 80.f };
    float masterDb = 0.f;
    bool  masterMute = false, eqOn = false;
    float bassDb = 0.f, trebleDb = 0.f, masterWidth = 1.f, limit = 0.f;
    int   velCurve = 1;        // 0 morbida, 1 normale, 2 dura
    bool  hhPedalCC4 = true;
    bool  setB[NumSetGroups] = {};
    float ohSpacing = 0.5f;    // 0 = stretti (40 cm) ... 1 = larghi (180 cm)
    float ohRibbon  = 0.f;     // 0 = condensatore a diaframma largo, 1 = nastro
    float hhMicX = 0.5f;       // 0 = verso il bordo, 1 = verso la campana
    float hhMicH = 0.25f;      // 0 = 3 cm ... 1 = 15 cm dal piatto
    bool  phaseAlign = true;   // ritarda i close perché arrivino insieme agli overhead
    int   roomType = 0;        // 0 studio, 1 big room
};

// -----------------------------------------------------------------------------
//  Filtri di base
// -----------------------------------------------------------------------------
struct OnePoleLP
{
    float a = 1.f, z = 0.f;
    void setCutoff (float hz, float sr) { a = 1.f - std::exp (-2.f * kPi * std::min (hz, 0.45f * sr) / sr); }
    inline float process (float x) { z += a * (x - z); return z; }
};

struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;

    inline float process (float x)
    {
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void reset() { z1 = z2 = 0; }

    void setHighShelf (float sr, float f, float gainDb)
    {
        float A = std::pow (10.f, gainDb / 40.f), w = 2 * kPi * f / sr, cs = std::cos (w), sn = std::sin (w);
        float alpha = sn / 2.f * std::sqrt (2.f);   // S = 1
        float sq = 2 * std::sqrt (A) * alpha;
        float a0 = (A + 1) - (A - 1) * cs + sq;
        set ((A * ((A + 1) + (A - 1) * cs + sq)) / a0, (-2 * A * ((A - 1) + (A + 1) * cs)) / a0,
             (A * ((A + 1) + (A - 1) * cs - sq)) / a0, (2 * ((A - 1) - (A + 1) * cs)) / a0,
             ((A + 1) - (A - 1) * cs - sq) / a0);
    }
    void setLowShelf (float sr, float f, float gainDb)
    {
        float A = std::pow (10.f, gainDb / 40.f), w = 2 * kPi * f / sr, cs = std::cos (w), sn = std::sin (w);
        float alpha = sn / 2.f * std::sqrt (2.f);
        float sq = 2 * std::sqrt (A) * alpha;
        float a0 = (A + 1) + (A - 1) * cs + sq;
        set ((A * ((A + 1) - (A - 1) * cs + sq)) / a0, (2 * A * ((A - 1) - (A + 1) * cs)) / a0,
             (A * ((A + 1) - (A - 1) * cs - sq)) / a0, (-2 * ((A - 1) + (A + 1) * cs)) / a0,
             ((A + 1) + (A - 1) * cs - sq) / a0);
    }
    void setPeak (float sr, float f, float q, float gainDb)
    {
        float A = std::pow (10.f, gainDb / 40.f), w = 2 * kPi * f / sr, cs = std::cos (w), alpha = std::sin (w) / (2 * q);
        float a0 = 1 + alpha / A;
        set ((1 + alpha * A) / a0, (-2 * cs) / a0, (1 - alpha * A) / a0, (-2 * cs) / a0, (1 - alpha / A) / a0);
    }
    void setHighPass (float sr, float f, float q = 0.707f)
    {
        float w = 2 * kPi * std::min (f, 0.45f * sr) / sr, cs = std::cos (w), alpha = std::sin (w) / (2 * q);
        float a0 = 1 + alpha;
        set (((1 + cs) / 2) / a0, (-(1 + cs)) / a0, ((1 + cs) / 2) / a0, (-2 * cs) / a0, (1 - alpha) / a0);
    }
    void setLowPass (float sr, float f, float q = 0.707f)
    {
        float w = 2 * kPi * std::min (f, 0.45f * sr) / sr, cs = std::cos (w), alpha = std::sin (w) / (2 * q);
        float a0 = 1 + alpha;
        set (((1 - cs) / 2) / a0, (1 - cs) / a0, ((1 - cs) / 2) / a0, (-2 * cs) / a0, (1 - alpha) / a0);
    }
private:
    void set (float nb0, float nb1, float nb2, float na1, float na2) { b0 = nb0; b1 = nb1; b2 = nb2; a1 = na1; a2 = na2; }
};

// Linea di ritardo semplice (potenza di due)
struct DelayLine
{
    std::vector<float> buf; int mask = 0, w = 0;
    void init (int maxSamples)
    {
        int n = 1; while (n < maxSamples + 2) n <<= 1;
        buf.assign ((size_t) n, 0.f); mask = n - 1; w = 0;
    }
    inline void push (float x) { buf[(size_t) w] = x; w = (w + 1) & mask; }
    inline float read (float d) const   // d in campioni (>= 1)
    {
        float rp = (float) w - d; int i = (int) std::floor (rp); float fr = rp - (float) i;
        float a = buf[(size_t) (i & mask)], b = buf[(size_t) ((i + 1) & mask)];
        return a + fr * (b - a);
    }
    void clear() { std::fill (buf.begin(), buf.end(), 0.f); }
};

// -----------------------------------------------------------------------------
//  Compressore di canale (feed-forward, veloce, adatto alle percussioni)
// -----------------------------------------------------------------------------
struct ChannelComp
{
    float env = 0.f, atk = 0.f, rel = 0.f, thr = 0.f, slope = 0.f, makeup = 1.f;
    bool  on = false;

    void set (float amount, float sr)
    {
        on = amount > 0.005f;
        atk = std::exp (-1.f / (0.002f * sr));
        rel = std::exp (-1.f / ((0.06f + 0.10f * amount) * sr));
        thr = -4.f - 26.f * amount;                 // soglia in dB
        const float ratio = 2.f + 6.f * amount;
        slope = 1.f - 1.f / ratio;
        makeup = dbToGain (0.45f * (-thr) * slope);
    }
    inline float process (float x)
    {
        if (! on) return x;
        const float a = std::abs (x);
        env = a > env ? atk * env + (1.f - atk) * a : rel * env + (1.f - rel) * a;
        const float envDb = 20.f * std::log10 (env + 1e-9f);
        const float gr = envDb > thr ? (thr - envDb) * slope : 0.f;
        return x * dbToGain (gr) * makeup;
    }
};

// -----------------------------------------------------------------------------
//  Limiter master (picco, senza lookahead) seguito da un soft clip
// -----------------------------------------------------------------------------
struct Limiter
{
    float env = 1.f, rel = 0.f;
    void prepare (float sr) { rel = std::exp (-1.f / (0.08f * sr)); env = 1.f; }
    inline void process (float& l, float& r, float ceiling)
    {
        const float pk = std::max (std::abs (l), std::abs (r));
        const float target = pk > ceiling ? ceiling / pk : 1.f;
        env = target < env ? target : rel * env + (1.f - rel) * target;
        l *= env; r *= env;
    }
};

// -----------------------------------------------------------------------------
//  Room: FDN a 8 linee con matrice di Householder e smorzamento
// -----------------------------------------------------------------------------
class Room
{
public:
    void prepare (double sr)
    {
        fs = (float) sr;
        float scale = fs / 48000.f;
        for (int i = 0; i < 8; ++i)
        {
            lines[i].init ((int) (kBase[i] * 1.6f * 2.4f * scale) + 8);   // spazio anche per la big room
            damp[i].z = 0;
        }
        pre.init ((int) (0.06f * fs) + 8);
        erLine.init ((int) (0.16f * fs) + 8);
        erLP[0].z = erLP[1].z = 0.f;
        erLP[0].setCutoff (6500.f, fs); erLP[1].setCutoff (6500.f, fs);
        erHP[0].reset(); erHP[1].reset(); erHP[0].setHighPass (fs, 160.f); erHP[1].setHighPass (fs, 160.f);
        hpL.reset(); hpR.reset(); lpL.reset(); lpR.reset();
        lpL.setLowPass (fs, 7500.f); lpR.setLowPass (fs, 7500.f);
        env = 0; gr = 1.f;
        lastSize = lastDecay = -1; lastType = -1;
    }

    // type: 0 = studio di registrazione (la stanza della foto), 1 = big room (sala grande e viva)
    void setParams (float size01, float rt60, float comp01, float locutHz, int type = 0)
    {
        if (std::abs (size01 - lastSize) > 1e-4f || std::abs (rt60 - lastDecay) > 1e-4f || type != lastType)
        {
            lastSize = size01; lastDecay = rt60; lastType = type;
            const bool big = type == 1;
            // la big room ha linee molto più lunghe (stanza grande) e una coda più lunga a parità di DECAY
            const float rt = big ? rt60 * 2.0f + 0.6f : rt60;
            float scale = fs / 48000.f * (0.55f + size01) * (big ? 2.2f : 1.f);
            for (int i = 0; i < 8; ++i)
            {
                len[i] = kBase[i] * scale;
                g[i] = std::pow (10.f, -3.f * len[i] / (rt * fs));
                // le alte decadono prima: molto in studio (legno e pannelli), meno nella sala grande (superfici dure)
                damp[i].setCutoff ((big ? 12000.f : 9000.f) - 4000.f * std::min (1.f, rt / 3.f), fs);
            }
            preDelay = (big ? 0.012f + 0.028f * size01 : 0.004f + 0.014f * size01) * fs;
            lpL.setLowPass (fs, big ? 9500.f : 7500.f); lpR.setLowPass (fs, big ? 9500.f : 7500.f);
            erGain = big ? 0.45f : kErGain;
            computeEarlyReflections (size01, big);
        }
        if (std::abs (locutHz - lastLocut) > 0.5f)
        {
            lastLocut = locutHz;
            hpL.setHighPass (fs, locutHz); hpR.setHighPass (fs, locutHz);
        }
        // compressore "schiacciato" tipico delle room
        threshDb = -6.f - 30.f * comp01;
        ratio    = 2.f + 8.f * comp01;
        makeupDb = 0.55f * (-threshDb) * (1.f - 1.f / ratio);
        atk = std::exp (-1.f / (0.004f * fs));
        rel = std::exp (-1.f / (0.12f * fs));
    }

    inline void process (float inL, float inR, float& outL, float& outR)
    {
        // prime riflessioni: microfoni room davanti al kit, sorgente al centro della batteria
        erLine.push (0.5f * (inL + inR));
        float erL = 0.f, erR = 0.f;
        for (int t = 0; t < numTaps; ++t)
        {
            erL += erLine.read (tapD[0][t]) * tapG[0][t];
            erR += erLine.read (tapD[1][t]) * tapG[1][t];
        }
        erL = erHP[0].process (erLP[0].process (erL));   // bassi solo dalla coda diffusa: niente tacche nel mix
        erR = erHP[1].process (erLP[1].process (erR));

        pre.push (0.5f * (inL + inR) + 0.25f * (erL + erR));
        float x  = pre.read (preDelay);
        float side = 0.5f * (inL - inR);

        float o[8], sum = 0.f;
        for (int i = 0; i < 8; ++i) { o[i] = damp[i].process (lines[i].read (len[i])) * g[i]; sum += o[i]; }
        float h = sum * (2.f / 8.f);
        for (int i = 0; i < 8; ++i)
        {
            float inj = (i & 1) ? (x - side) : (x + side);
            lines[i].push (o[i] - h + inj * kInGain[i]);
        }
        float l = 0.35f * (o[0] - o[2] + o[4] + o[6] - o[1] * 0.3f) + erGain * erL;
        float r = 0.35f * (o[1] - o[3] + o[5] + o[7] - o[0] * 0.3f) + erGain * erR;

        l = lpL.process (hpL.process (l));
        r = lpR.process (hpR.process (r));

        // compressore stereo collegato (peak/RMS veloce)
        float lvl = std::max (std::abs (l), std::abs (r));
        env = lvl > env ? atk * env + (1 - atk) * lvl : rel * env + (1 - rel) * lvl;
        float envDb = 20.f * std::log10 (env + 1e-9f);
        float grDb = envDb > threshDb ? (threshDb - envDb) * (1.f - 1.f / ratio) : 0.f;
        float m = dbToGain (grDb + makeupDb);
        outL = l * m; outR = r * m;
    }

    void clear()
    {
        for (auto& l : lines) l.clear();
        pre.clear();
        env = 0;
    }

private:
    // Modello a sorgenti immagine della stanza della foto (circa 5 x 4 x 3 m, legno e pannelli).
    // Calcola suono diretto + riflessioni di 1° e 2° ordine verso due microfoni room.
    void computeEarlyReflections (float size01, bool big)
    {
        const float k  = 0.7f + 0.7f * size01;                 // SIZE scala la stanza
        // studio: circa 5 x 4 x 3 m | big room: sala di circa 18 x 13 x 8 m
        const float Lx = (big ? 18.f : 5.0f) * k, Ly = (big ? 13.f : 4.0f) * k, Lz = (big ? 8.f : 3.0f) * k;
        const float src[3] = { 0.5f * Lx, (big ? 0.55f : 0.62f) * Ly, 0.9f };   // centro della batteria
        const float micY = big ? 0.55f * Ly - 4.5f : 0.2f * Ly, micZ = big ? 2.4f : 1.6f;   // in sala i room sono più lontani e più alti
        const float spread = big ? 1.5f : 0.8f;
        const float micX[2] = { 0.5f * Lx - spread, 0.5f * Lx + spread };
        // coefficienti di riflessione per asse. Studio: legno, pannelli, tappeto e soffitto a pannelli. Big room: superfici dure.
        const float refl[3] = { big ? 0.86f : 0.78f, big ? 0.80f : 0.62f, big ? 0.72f : 0.55f };
        const float c = 343.f;

        for (int m = 0; m < 2; ++m)
        {
            int n = 0;
            const float mic[3] = { micX[m], micY, micZ };
            const float L[3] = { Lx, Ly, Lz };
            // distanza del suono diretto (riferimento per il livello)
            float d0 = 0.f; for (int a = 0; a < 3; ++a) d0 += (src[a] - mic[a]) * (src[a] - mic[a]); d0 = std::sqrt (d0);

            for (int nx = -1; nx <= 1; ++nx) for (int px = 0; px <= 1; ++px)
            for (int ny = -1; ny <= 1; ++ny) for (int py = 0; py <= 1; ++py)
            for (int nz = -1; nz <= 1; ++nz) for (int pz = 0; pz <= 1; ++pz)
            {
                const int ord[3] = { std::abs (2 * nx - px), std::abs (2 * ny - py), std::abs (2 * nz - pz) };
                const int order = ord[0] + ord[1] + ord[2];
                if (order > 2 || n >= kMaxTaps) continue;
                const int nn[3] = { nx, ny, nz }, pp[3] = { px, py, pz };
                float d = 0.f, gain = 1.f;
                for (int a = 0; a < 3; ++a)
                {
                    const float img = 2.f * (float) nn[a] * L[a] + (pp[a] ? -src[a] : src[a]);
                    d += (img - mic[a]) * (img - mic[a]);
                    gain *= std::pow (refl[a], (float) ord[a]);
                }
                d = std::sqrt (d);
                const float delayS = d / c;
                if (delayS > 0.15f) continue;
                tapD[m][n] = std::max (1.f, delayS * fs);
                tapG[m][n] = gain * (d0 / d) * (order == 0 ? 0.55f : 1.f);   // il diretto un po' più basso
                ++n;
            }
            numTaps = std::min (numTaps == 0 ? n : numTaps, n);
            for (int t = n; t < kMaxTaps; ++t) { tapD[m][t] = 1.f; tapG[m][t] = 0.f; }
        }
        // normalizzo l'energia delle prime riflessioni
        for (int m = 0; m < 2; ++m)
        {
            float e = 0.f; for (int t = 0; t < kMaxTaps; ++t) e += tapG[m][t] * tapG[m][t];
            const float nrm = 1.f / std::sqrt (std::max (e, 1e-6f));
            for (int t = 0; t < kMaxTaps; ++t) tapG[m][t] *= nrm;
        }
        numTaps = kMaxTaps;
    }

    static constexpr int kMaxTaps = 32;
    static constexpr float kErGain = 0.32f;
    float erGain = kErGain;
    int lastType = -1;
    DelayLine erLine;
    OnePoleLP erLP[2];
    Biquad erHP[2];
    float tapD[2][kMaxTaps] {}, tapG[2][kMaxTaps] {};
    int numTaps = 0;

    static constexpr float kBase[8]   = { 1031.f, 1327.f, 1523.f, 1777.f, 1949.f, 2153.f, 2381.f, 2621.f };
    static constexpr float kInGain[8] = { 0.9f, -0.8f, 0.7f, -0.9f, 0.8f, -0.7f, 0.85f, -0.75f };
    DelayLine lines[8], pre;
    OnePoleLP damp[8];
    float len[8] {}, g[8] {};
    float fs = 48000.f, preDelay = 400.f, lastSize = -1, lastDecay = -1, lastLocut = -1;
    Biquad hpL, hpR, lpL, lpR;
    float env = 0, gr = 1, threshDb = -20, ratio = 4, makeupDb = 0, atk = 0.9f, rel = 0.999f;
};

// -----------------------------------------------------------------------------
//  Voce: riproduce un campione con velocity, intonazione, decay e choke
// -----------------------------------------------------------------------------
struct Voice
{
    bool active = false;
    int  piece = 0, slot = 0;
    const Sample* smp = nullptr;
    double pos = 0, rate = 1;
    float gain = 1;
    bool  useLP = false;
    OnePoleLP lp1, lp2;
    int   attackLen = 0, attackCount = 0;
    int   holdSamples = 0;  float decayMul = 1.f, env = 1.f;
    bool  choking = false;  float chokeMul = 1.f;
    uint32_t age = 0;

    inline float next()
    {
        const auto& d = smp->data;
        int i = (int) pos;
        if (i + 2 >= (int) d.size()) { active = false; return 0.f; }
        float fr = (float) (pos - i);
        // Hermite a 4 punti
        float xm1 = i > 0 ? d[(size_t) i - 1] : 0.f, x0 = d[(size_t) i], x1 = d[(size_t) i + 1], x2 = d[(size_t) i + 2];
        float c1 = 0.5f * (x1 - xm1), c2 = xm1 - 2.5f * x0 + 2.f * x1 - 0.5f * x2, c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        float s = ((c3 * fr + c2) * fr + c1) * fr + x0;
        pos += rate;

        if (useLP) s = lp2.process (lp1.process (s));

        float a = 1.f;
        if (attackCount < attackLen)
        {
            float t = (float) attackCount / (float) attackLen;
            a = t * t * (3.f - 2.f * t);
            ++attackCount;
        }
        if (holdSamples > 0) --holdSamples; else env *= decayMul;
        if (choking) { env *= chokeMul; }
        if (env < 1.0e-4f) active = false;
        return s * gain * a * env;
    }

    void choke (float ms, float sr)
    {
        if (! active) return;
        choking = true;
        chokeMul = std::exp (-1.f / (std::max (0.5f, ms) * 0.001f * sr / 4.6f));
    }
};

// -----------------------------------------------------------------------------
//  Motore
// -----------------------------------------------------------------------------
class Engine
{
public:
    static constexpr int kMaxVoices = 64;
    static constexpr int kMaxPerPiece = 8;

    std::array<Sample, NumSlots> samples;

    // misure per l'interfaccia
    std::atomic<float> meter[NumMeters];     // picco
    std::atomic<float> vu[NumMeters];        // RMS per i VU
    std::atomic<int>   hitCounter[NumPieces];
    std::atomic<int>   lastNote { -1 }, lastVel { 0 }, lastCC4 { 127 };

    Engine()
    {
        for (auto& m : meter) m.store (0.f);
        for (auto& m : vu) m.store (0.f);
        for (auto& h : hitCounter) h.store (0);
    }

    void prepare (double sampleRate, int maxBlock)
    {
        sr = (float) sampleRate;
        for (auto& v : voices) v.active = false;
        for (int p = 0; p < NumPieces; ++p)
        {
            chBuf[p].assign ((size_t) std::max (maxBlock, 256), 0.f);
            lastComp[p] = -1.f;
            ohDelay[p].init ((int) (0.009f * sr) + 8);
            closeDelay[p].init ((int) (0.009f * sr) + 8);
            ohOff[p][0].z = ohOff[p][1].z = 0.f;
            gainSm[p][0] = gainSm[p][1] = 0.f;
        }
        ohHP[0].reset(); ohHP[1].reset(); ohAir[0].reset(); ohAir[1].reset();
        lastOHLocut = lastOHAir = -1;
        room.prepare (sampleRate);
        for (int i = 0; i < 2; ++i) { busSm[0][i] = busSm[1][i] = 0.f; }
        masterSm = 0.f;
        for (int k = 0; k < 2; ++k)
        {
            bleedDelay[k].init ((int) (0.004f * sr) + 8);
            bleedHP[k].reset(); bleedLP[k].reset();
        }
        bleedHP[0].setHighPass (sr, 90.f);  bleedLP[0].setLowPass (sr, 2500.f);   // rullante nel mic della cassa
        bleedHP[1].setHighPass (sr, 70.f);  bleedLP[1].setLowPass (sr, 1800.f);   // cassa nel mic del rullante
        for (int k = 0; k < 2; ++k) { eqLow[k].reset(); eqHigh[k].reset(); }
        lastBass = lastTreble = 999.f;
        limiter.prepare (sr);
        micInit = false;
        for (int k = 0; k < 2; ++k) { ldcHP[k].reset(); ldcPres[k].reset(); ldcLP[k].reset(); ribLow[k].reset(); ribDip[k].reset(); ribLP[k].reset(); }
        kmHP.reset(); kmPres.reset(); hhBody.reset(); hhSizzle.reset(); hhProx.reset();
        hhBleedDelay.init ((int) (0.004f * sr) + 8); hhBleedHP.reset(); hhBleedLP.reset();
        lastHHX = lastHHH = -1.f; hhLevelSm = hhLevelT = 1.f; hhBleedSm = hhBleedT = 0.f; ribSm = params.ohRibbon;
    }

    void setParams (const EngineParams& p) { params = p; }

    // ---- MIDI --------------------------------------------------------------
    void noteOn (int note, int velocity)
    {
        float v = std::clamp (velocity / 127.f, 0.f, 1.f);
        lastNote.store (note); lastVel.store (velocity);
        int slot = -1;

        const bool* B = params.setB;
        switch (note)
        {
            case 35: case 36:                     slot = B[gKick]   ? sKickB   : sKick;  break;
            case 37:                              slot = B[gSnare]  ? sSnareB  : sSnare; v *= 0.35f; break;  // side stick
            case 38:                              slot = B[gSnare]  ? sSnareB  : sSnare; break;
            case 40:                              slot = sRimshot; break;                                   // rimshot
            case 41: case 43:                     slot = B[gFloor]  ? sFloorB  : sFloor; break;
            case 45: case 47: case 48: case 50:   slot = B[gTom]    ? sTomB    : sTom;   break;
            case 49: case 52:                     slot = B[gCrashL] ? sCrashLB : sCrashL; break;
            case 55: case 57:                     slot = B[gCrashR] ? sCrashRB : sCrashR; break;
            case 51: case 53: case 59:            slot = sRide;  break;
            case 42:                              slot = hhSlotFor (true);  break;
            case 46:                              slot = hhSlotFor (false); break;
            case 44:                              slot = B[gHiHat] ? sHHBPedal : sHHChick; break;
            case 21:                              slot = sHHSplash; break;
            default:
                if (note >= 22 && note <= 28)     // posizioni per apertura: 22 chiuso stretto ... 28 tutto aperto
                {
                    const int level = note - 22;
                    constexpr int bIdx[7] = { 3, 3, 2, 2, 1, 0, 0 };   // il set B ha 4 posizioni
                    slot = B[gHiHat] ? kHHOpennessB[bIdx[level]] : kHHOpenness[6 - level];
                }
                break;
        }
        if (slot >= 0) trigger (slot, v);
    }

    void controller (int cc, int value)
    {
        if (cc != 4) return;
        const float prev = cc4 / 127.f;
        cc4 = value; cc4Seen = true; lastCC4.store (value);
        const float now = cc4 / 127.f;
        // pedale che chiude: l'hi-hat aperto si spegne
        if (prev < 0.6f && now >= 0.6f)
            for (auto& v : voices)
                if (v.active && v.piece == HiHat && openness (v.slot) < 0.6f) v.choke (60.f, sr);
    }

    void polyPressure (int note, int value)
    {
        if (value < 64) return;   // choke dei piatti (afferrare il piatto)
        int piece = -1;
        if (note == 49 || note == 52) piece = CrashL;
        else if (note == 55 || note == 57) piece = CrashR;
        else if (note == 51 || note == 53 || note == 59) piece = Ride;
        if (piece >= 0) for (auto& v : voices) if (v.active && v.piece == piece) v.choke (90.f, sr);
    }

    void allNotesOff() { for (auto& v : voices) v.choke (30.f, sr); }

    // ---- Audio -------------------------------------------------------------
    void render (float* outL, float* outR, int n)
    {
        const int maxChunk = (int) chBuf[0].size();
        if (maxChunk <= 0) return;
        while (n > 0)
        {
            const int k = std::min (n, maxChunk);
            renderChunk (outL, outR, k);
            outL += k; outR += k; n -= k;
        }
    }

private:
    void renderChunk (float* outL, float* outR, int n)
    {

        for (int p = 0; p < NumPieces; ++p) std::fill (chBuf[p].begin(), chBuf[p].begin() + n, 0.f);

        for (auto& v : voices)
        {
            if (! v.active) continue;
            float* b = chBuf[v.piece].data();
            for (int i = 0; i < n && v.active; ++i) b[i] += v.next();
        }

        // compressori di canale
        for (int p = 0; p < NumPieces; ++p)
            if (std::abs (params.ch[p].comp - lastComp[p]) > 0.001f)
            {
                lastComp[p] = params.ch[p].comp;
                comp[p].set (lastComp[p], sr);
            }

        // OH
        {
            float lc = params.oh.p2, air = params.oh.p3;
            if (std::abs (lc - lastOHLocut) > 0.5f) { lastOHLocut = lc; ohHP[0].setHighPass (sr, lc); ohHP[1].setHighPass (sr, lc); }
            if (std::abs (air - lastOHAir) > 0.01f) { lastOHAir = air; ohAir[0].setHighShelf (sr, 10000.f, air); ohAir[1].setHighShelf (sr, 10000.f, air); }
        }
        room.setParams (params.room.p1, params.room.p2, params.room.p3, params.room.p4, params.roomType);

        // ---- curve dei microfoni (dai datasheet) -----------------------------
        if (! micInit)
        {
            micInit = true;
            for (int k = 0; k < 2; ++k)
            {
                // diaframma largo, cardioide: -3 dB a 40 Hz, presenza +2.5 dB a 9 kHz, -4 dB a 16 kHz
                ldcHP[k].setHighPass (sr, 35.f, 0.6f); ldcPres[k].setPeak (sr, 9000.f, 1.0f, 2.5f); ldcLP[k].setLowPass (sr, 16000.f, 0.6f);
                // nastro, figura 8: +3 dB sotto i 150 Hz, medi piatti, alti che scendono dolcemente (-10 dB a 20 kHz)
                ribLow[k].setLowShelf (sr, 150.f, 3.f); ribDip[k].setPeak (sr, 5500.f, 1.0f, -1.f); ribLP[k].setLowPass (sr, 13000.f, 0.55f);
            }
            // microfono a matita sull'hi-hat: bassi che scendono, +2 dB tra 6 e 10 kHz
            kmHP.setHighPass (sr, 60.f, 0.6f); kmPres.setPeak (sr, 8000.f, 0.9f, 2.f);
            hhBleedHP.setHighPass (sr, 150.f); hhBleedLP.setLowPass (sr, 6000.f);
        }
        if (std::abs (params.hhMicX - lastHHX) > 0.002f || std::abs (params.hhMicH - lastHHH) > 0.002f)
        {
            lastHHX = params.hhMicX; lastHHH = params.hhMicH;
            hhBody.setPeak (sr, 700.f, 0.8f, -1.5f + 4.f * lastHHX);      // verso la campana: più corpo
            hhSizzle.setHighShelf (sr, 7000.f, 2.5f - 5.f * lastHHX);     // verso il bordo: più sfrigolio
            hhProx.setLowShelf (sr, 200.f, 4.f * (1.f - lastHHH));        // vicino: effetto di prossimità
            hhLevelT = dbToGain (1.f - 4.f * lastHHH);
            hhBleedT = 0.02f + 0.10f * lastHHH;                           // lontano: più rullante nel microfono
        }

        // ---- geometria degli overhead ---------------------------------------
        // due microfoni a distanza D, 1 m sopra i pezzi, puntati verso il centro del kit
        {
            const float D = 0.4f + 1.4f * params.ohSpacing, hgt = 1.0f;
            for (int p = 0; p < NumPieces; ++p)
            {
                const float xp = kTraits[p].kitPos * 0.9f;
                for (int m = 0; m < 2; ++m)
                {
                    const float xm = (m == 0 ? -0.5f : 0.5f) * D;
                    const float vx = xp - xm, vy = -hgt, Lp = std::sqrt (vx * vx + vy * vy);
                    const float ax = -xm, ay = -hgt, al = std::sqrt (ax * ax + ay * ay);
                    const float cosT = std::clamp ((vx * ax + vy * ay) / (Lp * al), -1.f, 1.f);
                    ohPol[p][m][0] = (0.5f + 0.5f * cosT) / Lp;   // cardioide
                    ohPol[p][m][1] = std::abs (cosT) / Lp;        // figura 8
                    ohDel[p][m] = (Lp / 343.f) * sr + 1.f;
                    const float sin2 = 1.f - cosT * cosT;          // fuori asse gli alti calano
                    ohOff[p][m].setCutoff (19000.f * (1.f - 0.72f * sin2), sr);
                }
                alignD[p] = params.phaseAlign ? kAlignFrac[p] * (0.5f * (ohDel[p][0] + ohDel[p][1]) - 1.f) : 0.f;
            }
        }

        // EQ master
        if (std::abs (params.bassDb - lastBass) > 0.01f)
        {
            lastBass = params.bassDb;
            for (auto& f : eqLow) f.setLowShelf (sr, 100.f, lastBass);
        }
        if (std::abs (params.trebleDb - lastTreble) > 0.01f)
        {
            lastTreble = params.trebleDb;
            for (auto& f : eqHigh) f.setHighShelf (sr, 8000.f, lastTreble);
        }

        bool anySolo = params.oh.solo || params.room.solo;
        for (int p = 0; p < NumPieces; ++p) anySolo = anySolo || (hasCloseChannel (p) && params.ch[p].solo);

        float peak[NumMeters] = {}, sq[NumMeters] = {};

        const float ohGainT   = audible (params.oh.mute, params.oh.solo, anySolo) ? dbToGain (params.oh.levelDb) : 0.f;
        const float roomGainT = audible (params.room.mute, params.room.solo, anySolo) ? dbToGain (params.room.levelDb) : 0.f;
        // -2 dB di margine: con l'allineamento di fase i transienti si sommano in modo più coerente
        const float masterT   = params.masterMute ? 0.f : dbToGain (params.masterDb - 2.f);
        const float width     = params.oh.p1;
        const float mWidth    = params.masterWidth;
        const float drive     = dbToGain (12.f * params.limit);
        const float ceiling   = dbToGain (-0.5f);
        const float sm = 1.f - std::exp (-1.f / (0.015f * sr));   // smoothing ~15 ms

        float chGainT[NumPieces][2];
        float ohAmt[NumPieces], roomL[NumPieces], roomR[NumPieces];
        for (int p = 0; p < NumPieces; ++p)
        {
            const auto& c = params.ch[p];
            float g = (hasCloseChannel (p) && audible (c.mute, c.solo, anySolo)) ? dbToGain (c.levelDb) : 0.f;
            float a = (c.pan + 1.f) * 0.25f * kPi;            // pan a potenza costante
            chGainT[p][0] = g * std::cos (a) * 1.4142f;
            chGainT[p][1] = g * std::sin (a) * 1.4142f;

            const auto& tr = kTraits[p];
            ohAmt[p] = tr.ohCapture * c.ohSend * 1.05f;
            float rm = tr.roomCapture * c.roomSend;
            roomL[p] = rm * (0.8f - 0.2f * tr.kitPos);
            roomR[p] = rm * (0.8f + 0.2f * tr.kitPos);
        }
        const float hhBleedD = 0.0012f * sr;

        // bleed: quantità 0..1 -> fino a -6 dB, curva quadratica
        const float bleedKick  = 0.5f * params.ch[Kick].bleed  * params.ch[Kick].bleed;
        const float bleedSnare = 0.5f * params.ch[Snare].bleed * params.ch[Snare].bleed;
        const float bleedD = 0.0018f * sr;   // ~60 cm di distanza

        for (int i = 0; i < n; ++i)
        {
            float L = 0, R = 0, ohL = 0, ohR = 0, rmL = 0, rmR = 0;

            const float rawKick = chBuf[Kick][(size_t) i], rawSnare = chBuf[Snare][(size_t) i];
            bleedDelay[0].push (rawSnare);
            bleedDelay[1].push (rawKick);
            const float snareInKick = bleedLP[0].process (bleedHP[0].process (bleedDelay[0].read (bleedD))) * bleedKick;
            const float kickInSnare = bleedLP[1].process (bleedHP[1].process (bleedDelay[1].read (bleedD))) * bleedSnare;
            hhBleedDelay.push (rawSnare);
            hhLevelSm += sm * (hhLevelT - hhLevelSm);
            hhBleedSm += sm * (hhBleedT - hhBleedSm);
            ribSm     += sm * 0.3f * (params.ohRibbon - ribSm);   // dissolvenza ~50 ms tra i microfoni
            const float snareInHH = hhBleedLP.process (hhBleedHP.process (hhBleedDelay.read (hhBleedD))) * hhBleedSm;

            for (int p = 0; p < NumPieces; ++p)
            {
                const float raw = chBuf[p][(size_t) i];
                ohDelay[p].push (raw);
                closeDelay[p].push (raw);

                // allineamento di fase: il close arriva insieme alla media dei due overhead
                float close = alignD[p] >= 1.f ? closeDelay[p].read (alignD[p]) : raw;
                if (p == Kick)  close += snareInKick;
                if (p == Snare) close += kickInSnare;
                if (p == HiHat)   // il canale hi-hat passa dal microfono a matita, nella posizione scelta
                {
                    float hx = kmPres.process (kmHP.process (close + snareInHH));
                    close = hhSizzle.process (hhBody.process (hhProx.process (hx))) * hhLevelSm;
                }
                const float x = comp[p].process (close);

                gainSm[p][0] += sm * (chGainT[p][0] - gainSm[p][0]);
                gainSm[p][1] += sm * (chGainT[p][1] - gainSm[p][1]);
                const float l = x * gainSm[p][0], r = x * gainSm[p][1];
                L += l; R += r;
                const float pk = std::max (std::abs (l), std::abs (r));
                peak[p] = std::max (peak[p], pk); sq[p] += 0.5f * (l * l + r * r);

                for (int m = 0; m < 2; ++m)
                {
                    const float pol = (1.f - ribSm) * ohPol[p][m][0] + ribSm * ohPol[p][m][1];
                    const float sOH = ohOff[p][m].process (ohDelay[p].read (ohDel[p][m])) * pol * ohAmt[p];
                    if (m == 0) ohL += sOH; else ohR += sOH;
                }
                rmL += raw * roomL[p];
                rmR += raw * roomR[p];
            }
            float roL, roR;
            room.process (rmL, rmR, roL, roR);

            // OH: curva del microfono scelto, poi low cut, aria, larghezza M/S
            {
                const float cL = ldcLP[0].process (ldcPres[0].process (ldcHP[0].process (ohL)));
                const float cR = ldcLP[1].process (ldcPres[1].process (ldcHP[1].process (ohR)));
                const float rL = ribLP[0].process (ribDip[0].process (ribLow[0].process (ohL)));
                const float rR = ribLP[1].process (ribDip[1].process (ribLow[1].process (ohR)));
                // il nastro (figura 8) capta anche dietro: un po' di stanza negli overhead
                ohL = (1.f - ribSm) * cL + ribSm * (rL + 0.22f * roL);
                ohR = (1.f - ribSm) * cR + ribSm * (rR + 0.22f * roR);
            }
            ohL = ohAir[0].process (ohHP[0].process (ohL));
            ohR = ohAir[1].process (ohHP[1].process (ohR));
            float mid = 0.5f * (ohL + ohR), side = 0.5f * (ohL - ohR) * width;
            ohL = mid + side; ohR = mid - side;
            busSm[0][0] += sm * (ohGainT - busSm[0][0]);
            ohL *= busSm[0][0]; ohR *= busSm[0][0];
            peak[MeterOH] = std::max (peak[MeterOH], std::max (std::abs (ohL), std::abs (ohR)));
            sq[MeterOH] += 0.5f * (ohL * ohL + ohR * ohR);

            busSm[1][0] += sm * (roomGainT - busSm[1][0]);
            roL *= busSm[1][0]; roR *= busSm[1][0];
            peak[MeterRoom] = std::max (peak[MeterRoom], std::max (std::abs (roL), std::abs (roR)));
            sq[MeterRoom] += 0.5f * (roL * roL + roR * roR);

            // master: EQ -> width -> limit -> fader -> soft clip
            float mL = L + ohL + roL, mR = R + ohR + roR;
            if (params.eqOn)
            {
                mL = eqHigh[0].process (eqLow[0].process (mL));
                mR = eqHigh[1].process (eqLow[1].process (mR));
            }
            {
                const float m = 0.5f * (mL + mR), sd = 0.5f * (mL - mR) * mWidth;
                mL = m + sd; mR = m - sd;
            }
            if (params.limit > 0.005f)
            {
                mL *= drive; mR *= drive;
                limiter.process (mL, mR, ceiling);
            }
            masterSm += sm * (masterT - masterSm);
            const float oL = softClip (mL * masterSm);
            const float oR = softClip (mR * masterSm);
            outL[i] = oL; outR[i] = oR;
            peak[MeterMaster] = std::max (peak[MeterMaster], std::max (std::abs (oL), std::abs (oR)));
            sq[MeterMaster] += 0.5f * (oL * oL + oR * oR);
        }

        for (int m = 0; m < NumMeters; ++m)
        {
            meterPush (meter[m], peak[m]);
            meterPush (vu[m], std::sqrt (sq[m] / (float) n));
        }
    }

    static bool audible (bool mute, bool solo, bool anySolo) { return ! mute && (! anySolo || solo); }

    static void meterPush (std::atomic<float>& m, float v)
    {
        float cur = m.load (std::memory_order_relaxed);
        if (v > cur) m.store (v, std::memory_order_relaxed);   // l'editor lo legge e lo fa scendere
    }

    static inline float softClip (float x)
    {
        const float k = 0.89f;
        float a = std::abs (x);
        if (a <= k) return x;
        float over = (a - k) / (1.f - k);
        float y = k + (1.f - k) * over / (1.f + over);   // arriva dolcemente verso 1.0
        return x < 0 ? -y : y;
    }

    // apertura normalizzata: 0 = aperto, 1 = chiuso
    static float openness (int slot)
    {
        for (int i = 0; i < 7; ++i) if (kHHOpenness[i] == slot) return (float) i / 6.f;
        for (int i = 0; i < 4; ++i) if (kHHOpennessB[i] == slot) return (float) i / 3.f;
        if (slot == sHHChick || slot == sHHBPedal) return 1.f;
        return 0.f;   // splash
    }

    int hhSlotFor (bool closedNote) const
    {
        const bool b = params.setB[gHiHat];
        const int n = b ? 4 : 7;
        int idx;
        if (params.hhPedalCC4 && cc4Seen) idx = std::clamp ((int) std::lround (cc4 / 127.0 * (n - 1)), 0, n - 1);
        else idx = closedNote ? (b ? 3 : 5) : 0;
        return b ? kHHOpennessB[idx] : kHHOpenness[idx];
    }

    float rnd()   // -1..1
    {
        rng = rng * 1664525u + 1013904223u;
        return (float) ((rng >> 8) & 0xFFFF) / 32767.5f - 1.f;
    }

    void trigger (int slot, float v)
    {
        const Sample& s = samples[(size_t) slot];
        if (s.data.size() < 8) return;
        const int piece = s.piece;
        const auto& tr = kTraits[piece];
        const auto& c = params.ch[piece];

        // gruppo hi-hat: ogni nuovo colpo spegne quello precedente
        if (piece == HiHat)
            for (auto& v2 : voices) if (v2.active && v2.piece == HiHat) v2.choke ((slot == sHHChick || slot == sHHBPedal) ? 35.f : 25.f, sr);

        // limite di voci per pezzo: la più vecchia sfuma
        int count = 0; Voice* oldest = nullptr;
        for (auto& v2 : voices)
            if (v2.active && v2.piece == piece && ! v2.choking)
            {
                ++count;
                if (! oldest || v2.age < oldest->age) oldest = &v2;
            }
        if (count >= kMaxPerPiece && oldest) oldest->choke (8.f, sr);

        Voice* voice = nullptr;
        for (auto& v2 : voices) if (! v2.active) { voice = &v2; break; }
        if (! voice)
        {
            for (auto& v2 : voices) if (! voice || v2.age < voice->age) voice = &v2;
        }

        const float curveK[3] = { 1.1f, 1.7f, 2.5f };
        float k = curveK[std::clamp (params.velCurve, 0, 2)];
        float amp = std::pow (std::max (v, 0.008f), k);

        Voice& nv = *voice;
        nv = Voice();
        nv.active = true; nv.piece = piece; nv.slot = slot; nv.smp = &s;
        nv.age = ++ageCounter;
        nv.gain = amp * dbToGain (s.baseGainDb + rnd() * 0.5f);

        float cents = c.tune * 100.f + tr.velPitchCents * (v - 0.5f) * 2.f + rnd() * 3.f;
        nv.rate = std::pow (2.0, cents / 1200.0) * s.sampleRate / sr;

        // brillantezza in funzione della velocity
        float bright = std::clamp (v + rnd() * 0.03f, 0.f, 1.f);
        if (bright < 0.97f)
        {
            float maxHz = std::min (20000.f, 0.45f * sr);
            float hz = tr.brightMinHz * std::pow (maxHz / tr.brightMinHz, std::pow (bright, 0.75f));
            nv.useLP = true;
            nv.lp1.setCutoff (hz * 1.3f, sr); nv.lp2.setCutoff (hz * 1.3f, sr);
        }
        // attacco più morbido nei colpi leggeri
        float atkMs = tr.attackMaxMs * (1.f - v) * (1.f - v);
        nv.attackLen = (int) (atkMs * 0.001f * sr);

        // decay del canale
        if (c.decay < 0.995f)
        {
            float lenSec = (float) s.data.size() / (float) s.sampleRate;
            float hold = std::max (0.015f, c.decay * lenSec * 0.30f);
            float tau  = std::max (0.010f, c.decay * lenSec * 0.18f);
            nv.holdSamples = (int) (hold * sr);
            nv.decayMul = std::exp (-1.f / (tau * sr));
        }

        hitCounter[piece].fetch_add (1, std::memory_order_relaxed);
    }

    float sr = 48000.f;
    EngineParams params;
    std::array<Voice, kMaxVoices> voices;
    std::vector<float> chBuf[NumPieces];
    ChannelComp comp[NumPieces]; float lastComp[NumPieces] {};
    DelayLine bleedDelay[2]; Biquad bleedHP[2], bleedLP[2];
    Biquad eqLow[2], eqHigh[2]; float lastBass = 999.f, lastTreble = 999.f;
    Limiter limiter;
    // microfoni
    bool micInit = false;
    Biquad ldcHP[2], ldcPres[2], ldcLP[2], ribLow[2], ribDip[2], ribLP[2];
    Biquad kmHP, kmPres, hhBody, hhSizzle, hhProx, hhBleedHP, hhBleedLP;
    DelayLine hhBleedDelay;
    float lastHHX = -1.f, lastHHH = -1.f, hhLevelT = 1.f, hhLevelSm = 1.f, hhBleedT = 0.f, hhBleedSm = 0.f, ribSm = 0.f;
    float ohPol[NumPieces][2][2] {}, ohDel[NumPieces][2] {};
    OnePoleLP ohOff[NumPieces][2];
    DelayLine closeDelay[NumPieces];
    float alignD[NumPieces] {};
    DelayLine ohDelay[NumPieces];
    float gainSm[NumPieces][2] {};
    Biquad ohHP[2], ohAir[2]; float lastOHLocut = -1, lastOHAir = -1;
    Room room;
    float busSm[2][2] {}, masterSm = 0;
    int cc4 = 127; bool cc4Seen = false;
    uint32_t rng = 22222u, ageCounter = 0;
};

} // namespace md
