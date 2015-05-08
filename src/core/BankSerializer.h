
#pragma once

#include <string>
#include "core/Preset.h"


namespace juce {
    class XmlElement;
}

namespace e3 {

    class Bank;
    class Instrument;
    class Module;
    class Parameter;
    class Link;

    using juce::XmlElement;


    class BankSerializer
    {
    public:
        static XmlElement* readBank( const std::string& path );
        static XmlElement* createNewBank();
        static void saveBank( const std::string& path, XmlElement* root );

        static Instrument* loadInstrument( XmlElement* root, int id );
        static void saveInstrument( XmlElement* root, Instrument* instrument );
        static void saveInstrumentAttributes( XmlElement* root, Instrument* instrument );
        static void saveInstrumentAttribute( XmlElement* root, int id, const std::string& name, const var value );
        static void saveInstrumentLinks( XmlElement* root, Instrument* instrument );

    protected:
        static void readInstrument( XmlElement* e, Instrument* instrument );
        static void readInstrumentAttributes( XmlElement* e, Instrument* instrument );
        static void readInstrumentModules( XmlElement* e, Instrument* instrument );
        static void readInstrumentLinks( XmlElement* e, Instrument* instrument );
        static void readInstrumentPresets( XmlElement* e, Instrument* instrument );
        static void readParameters( XmlElement* e, Module* module );
        static void readParameter( XmlElement* e, Parameter& p );

        static void writeInstrumentAttributes( XmlElement* e, Instrument* instrument );
        static void writeInstrumentModules( XmlElement* e, Instrument* instrument );
        static void writeInstrumentLinks( XmlElement* e, Instrument* instrument );
        static void writeInstrumentPresets( XmlElement* e, Instrument* instrument );

        static void writeModule( XmlElement* e, Module* module );
        static void writePreset( XmlElement* e, Instrument* instrument, const Preset& preset );

        static void writeParameters( XmlElement* e, Module* module );
        static void writeParameter( XmlElement* e, const Parameter& param, const Parameter& defaultParam );
        static void writeLink( XmlElement* e, const Link& link );

        static XmlElement* getChildElement( XmlElement* e, const std::string& name );
        static XmlElement* getAndClearChildElement( XmlElement* e, const std::string& name );

        static File checkPath( const std::string& path );
        static void checkRoot( XmlElement* root );

        static std::string defaultBankXml;
    };


} // namespace e3