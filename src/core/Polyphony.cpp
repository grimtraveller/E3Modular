
#include <e3_Trace.h>
#include "core/GlobalHeader.h"
#include "core/Polyphony.h"


namespace e3 {

    Polyphony::Polyphony()
    {
        setNumVoices(1);
        stack_.reserve(100);
        monitorVoiceEvent(0);
    }


    void Polyphony::handleMidiMessage(const MidiMessage& m)
    {
        const uint8_t* data = m.getRawData();
        TRACE("Midi Message: [0]: %3d   [1]: %3d   [2]: %3d \n", data[0], data[1], data[2]);
        //TRACE("Midi Message: [0]: %2x [1]: %2x [2]: %2x \n", data[0], data[1], data[2]);

        if (m.isNoteOn())
        {
            noteOn(m.getNoteNumber(), m.getFloatVelocity());
        }
        else if (m.isNoteOff())
        {
            noteOff(m.getNoteNumber());
        }
        else if (m.isAllNotesOff() || m.isAllSoundOff())
        {
            allNotesOff(true);
        }
        else if (m.isPitchWheel())
        {
            int value = m.getPitchWheelValue();
            midiPitchbendSignal(value);
            monitorPitchbendEvent(value);
        }
        else if (m.isAftertouch())
        {
            midiAftertouchSignal(m.getNoteNumber(), m.getAfterTouchValue());
        }
        else if (m.isController())
        {
            int number = m.getControllerNumber();
            int value  = m.getControllerValue();
            midiControllerSignal(number, value);
            monitorControllerEvent(number, value);
        }
    }


    void Polyphony::noteOn(double pitch, double gate)
    {
        monitorNoteEvent(pitch, gate);

        gate /= ((numUnison_ > 1) ? (numUnison_ * 0.66f) : 1);
        int unisonGroup = tags_;

        double basePitch = pitch + tuning_;

        if (hold_ && numActive_ == 0 && !sustain_) {
            allNotesOff(false);
        }

        for (uint16_t i = 0; i < numUnison_; i++)   // trigger unison voices
        {
            uint16_t voice = getUnusedVoice();
            if (voice >= 0)
            {
                double unisonPitch    = basePitch + i * unisonSpread_;
                Voice* current        = &voices_[voice];
                current->unisonGroup_ = unisonGroup;
                current->pitch_       = unisonPitch;

                if (i == 0) {  // put the played note on the stack
                    stack_.push_back(Voice(voice, Voice::kNoteOn, unisonPitch, gate, tags_, unisonGroup));
                }
                startVoice(voice, unisonPitch, gate);
            }
        }
        combineVoices(basePitch, gate);
        lastPitch_ = basePitch;
    }

    
    void Polyphony::noteOff(double pitch)
    {
        double basePitch = pitch + tuning_;

        int idxStack = -1;
        Voice* offVoice = nullptr;

        for (idxStack = stack_.size() - 1; idxStack >= 0; idxStack--)    // find voice
        {
            if (stack_[idxStack].pitch_ == basePitch) {
                offVoice = &stack_[idxStack];
                break;
            }
        }
        if (nullptr == offVoice) {
            return;
        }

        if (retrigger_ && stack_.size() >= 2)                      // retrigger remaining voices
        {
            Voice* prev = &stack_.at(stack_.size() - 2);

            if (prev->tag_ != offVoice->tag_)
            {
                lastPitch_ = offVoice->pitch_;
                combineVoices(prev->pitch_, -1);
                lastPitch_ = prev->pitch_;
            }
        }

        for (uint16_t i = 0; i<numVoices_; i++)                       // set state to NoteOff for the voice and all assigned unisonVoices
        {
            Voice* next = &voices_[i];
            if (next->unisonGroup_ == offVoice->unisonGroup_ && next->state_ > Voice::kSilent)
            {
                next->state_ &= ~Voice::kNoteOn;
                next->state_ |= Voice::kNoteOff;
                numActive_--;
                ASSERT(numActive_ >= 0);

                if (!hold_ && !sustain_ || (hold_ && retrigger_ && !sustain_ && numActive_ >= numUnison_))  {
                    midiGateSignal(0, next->id_);
                    midiNoteSignal(next->pitch_, 0, next->id_);
                }
            }
        }
        stack_.erase(stack_.begin() + idxStack);                    // remove note from stack
    }

    
    void Polyphony::startVoice(uint16_t voice, double pitch, double gate)
    {
        uint16_t state = Voice::kNoteOn | (hold_ ? Voice::kNoteHold : 0);
        voices_[voice].init(voice, state, pitch, gate, tags_);

        midiPitchSignal(pitch, voice);
        midiGateSignal(gate, voice);
        midiNoteSignal(pitch, gate, voice);

        tags_++;
        numSounding_++;
        numActive_++;
        updateSoundingVoices();
        monitorVoiceEvent(numSounding_);
        ASSERT(numSounding_ <= (int16_t)numVoices_);
    }


    void Polyphony::endVoice(uint16_t voice)
    {
        Voice& v = voices_[voice];
        if (v.state_)
        {
            numSounding_--;
            monitorVoiceEvent(numSounding_);

            v.reset();
            updateSoundingVoices();
        }
        ASSERT(numSounding_ >= 0);
        //if (numSounding_ == 0)
            //synth_->endProfiling();
    }
    
    
    void Polyphony::allNotesOff(bool reset)
    {
        for (size_t i = 0; i<voices_.size(); i++)
        {
            Voice* next = &voices_[i];
            if (reset) {
                next->reset();
            }
            else if (next->state_ > Voice::kSilent)
            {
                next->state_ = Voice::kNoteOff;
                midiGateSignal(0, next->id_);
                midiNoteSignal(next->pitch_, 0, next->id_);
            }
        }
        stack_.clear();

        if (reset) numSounding_ = 0;
        numActive_ = 0;

        updateSoundingVoices();
        monitorVoiceEvent(numSounding_);
    }


    void Polyphony::setNumVoices(uint16_t numVoices)
    {
        stack_.clear();
        voices_.clear();
        voices_.resize(numVoices);

        numVoices_   = numVoices;
        numSounding_ = 0;
        numActive_   = 0;

        soundingVoices_.resize(numVoices);
        updateSoundingVoices();
    }


    void Polyphony::combineVoices(double basePitch, double gate)
    {
        if (retrigger_ || legato_)
        {
            gate = legato_ ? -1.f : gate;

            for (size_t i = 0; i<stack_.size(); i++)                 // iterate over all played voices
            {
                Voice* first = &stack_[i];
                uint16_t counter = 0;
                for (uint16_t j = 0; j<numVoices_ && counter < numUnison_; j++)    // iterate over unison voices
                {
                    Voice* next = &voices_[j];
                    if (next->unisonGroup_ == first->unisonGroup_ && next->state_ > Voice::kSilent)
                    {
                        double pitch = basePitch + counter * unisonSpread_;

                        if (next->pitch_ != pitch)
                        {
                            next->pitch_ = pitch;
                            midiNoteSignal(pitch, gate, j);
                        }
                        counter++;
                    }
                }
            }
        }
    }
    
    
    void Polyphony::updateSoundingVoices()
    {
        uint16_t counter = 0;

        for (uint16_t i = 0; i < numVoices_; i++)
        {
            if (voices_[i].state_) {
                soundingVoices_[counter++] = i;
            }
        }
    }


    int16_t Polyphony::getUnusedVoice()
    {
        int16_t idxKill = -1;

        for (uint16_t i = 0; i < numVoices_; i++)
        {
            if (voices_[i].state_ == Voice::kSilent) {         // voices available?
                return i;
            }
        }

        for (uint16_t i = 0; i < numVoices_; i++)		       // all voices sounding, so interrupt oldest.
        {
            if (idxKill == -1) {
                idxKill = i;
            }
            else if (voices_[i].tag_ < voices_[idxKill].tag_) {
                idxKill = i;
            }
        }
        if (idxKill >= 0) {
            endVoice(idxKill);
        }
        return idxKill;
    }

} // namespace e3