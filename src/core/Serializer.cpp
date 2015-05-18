
#include "JuceHeader.h"
#include <e3_Trace.h>

#include "core/Instrument.h"
#include "core/Database.h"
#include "core/Serializer.h"


namespace e3 {

    std::string Serializer::defaultBankXml =
        "<bank name='New Bank' instrument='0'>"
        "<instrument name='Empty Instrument' id='0' preset='0' category='' comment='' voices='32'>"
        "<modules />"
        "<links />"
        "<panels/>"
        "<presets />"
        "</instrument>"
        "</bank>";


    XmlElement* Serializer::loadBank( const std::string& path )
    {
        File file = checkPath( path );
        XmlElement* root = XmlDocument::parse( file );

        if (root == nullptr || root->hasTagName( "bank" ) == false) {
            if (root) delete root;
            THROW( std::runtime_error, "Invalid bank file" );
        }

        return root;
    }


    XmlElement* Serializer::loadDatabase( File& file )
    {
        checkPath( file );
        XmlElement* root = XmlDocument::parse( file );

        if (root == nullptr || root->hasTagName( "e3m-database" ) == false) {
            if (root) delete root;
            root = nullptr;
        }
        if (root == nullptr) {
            root = new XmlElement( "e3m-database" );
        }
        return root;
    }


    bool Serializer::storeDatabase( XmlElement* xml, const File& file )
    {
        return xml != nullptr ? xml->writeToFile( file, "", "UTF-8", 1000 ) : false;
    }


    Instrument* Serializer::loadInstrument( const std::string& path )
    {
        File file = checkPath( path );
        XmlElement* root = XmlDocument::parse( file );

        if (root != nullptr)
        {
            try {
                Instrument* instrument = new Instrument( root, path );  // processor will be owner of the instrument and delete it
                
                readInstrumentAttributes( root, instrument );
                readInstrumentModules( root, instrument );
                readInstrumentLinks( root, instrument );
                readInstrumentPresets( root, instrument );

                return instrument;
            }
            catch (const std::exception& e) {               // parse error, skip instrument
                TRACE( e.what() );
            }
        }
        return nullptr;
    }


    void Serializer::scanInstrument( XmlElement* databaseXml, const File& file )
    {
        XmlElement* instrumentXml = nullptr;
        ScopedPointer<XmlElement> root = XmlDocument::parse( file );

        if (root != nullptr)
        {
            try {
                if (databaseXml == nullptr) {
                    databaseXml = new XmlElement( "e3m_database" );
                }
                instrumentXml = new XmlElement( "instrument" );
                instrumentXml->setAttribute( "path", file.getFullPathName() );

                XmlElement* presetsXml = getChildElement( root, "presets" );

                forEachXmlChildElementWithTagName( *presetsXml, e, "preset" )
                {
                    XmlElement* presetXml = new XmlElement( "preset" );
                    presetXml->setAttribute( "id", e->getIntAttribute( "id" ));
                    presetXml->setAttribute( "name", e->getStringAttribute( "name" ) );
                    presetXml->setAttribute( "category1", e->getStringAttribute( "category1" ) );
                    presetXml->setAttribute( "category2", e->getStringAttribute( "category2" ) );
                    presetXml->setAttribute( "category3", e->getStringAttribute( "category3" ) );

                    instrumentXml->addChildElement( presetXml );
                }
            }
            catch (const std::exception& e) {  // parse error
                TRACE( e.what() );
                if (instrumentXml != nullptr) delete instrumentXml;
                return;
            }
            databaseXml->addChildElement( instrumentXml );
        }
    }



    XmlElement* Serializer::createNewBank()
    {
        return XmlDocument::parse( defaultBankXml );
    }


    void Serializer::saveBank( const std::string& path, XmlElement* root )
    {
        if (root->writeToFile( File( path ), "", "UTF-8", 1000 ) == false) {
            THROW( std::runtime_error, "error writing bank file" );
        }
    }


    Instrument* Serializer::loadInstrument( XmlElement* root, int id )
    {
        XmlElement* e = root->getChildByAttribute( "id", String( id ) );
        if (e != nullptr)
        {
            try {
                Instrument* instrument = new Instrument();  // processor will be owner of the instrument and delete it
                readInstrumentAttributes( e, instrument );
                readInstrumentModules( e, instrument );
                readInstrumentLinks( e, instrument );
                readInstrumentPresets( e, instrument );
                return instrument;
            }
            catch (const std::exception& e) {               // parse error, skip instrument
                TRACE( e.what() );
            }
        }
        return nullptr;
    }


    void Serializer::saveInstrument( XmlElement* root, Instrument* instrument )
    {
        XmlElement* e = root->getChildByAttribute( "id", String( instrument->id_ ) );
        if (e != nullptr)
        {
            try {
                writeInstrumentAttributes( e, instrument );
                writeInstrumentModules( e, instrument );
                writeInstrumentLinks( e, instrument );
                writeInstrumentPresets( e, instrument );
            }
            catch (const std::exception& e) {       // parse error
                TRACE( e.what() );
            }
        }
    }


    void Serializer::saveInstrumentAttributes( XmlElement* root, Instrument* instrument )
    {
        XmlElement* e = root->getChildByAttribute( "id", String( instrument->id_ ) );
        if (e != nullptr)
        {
            try {
                writeInstrumentAttributes( e, instrument );
            }
            catch (const std::exception& e) {       // xml error
                TRACE( e.what() );
            }
        }
    }


    void Serializer::saveInstrumentAttribute( XmlElement* root, int id, const std::string& name, const var value )
    {
        XmlElement* e = root->getChildByAttribute( "id", String( id ) );
        if (e != nullptr)
        {
            e->setAttribute( Identifier(name), value.toString() );
        }
    }


    void Serializer::saveInstrumentLinks( XmlElement* root, Instrument* instrument )
    {
        XmlElement* e = root->getChildByAttribute( "id", String( instrument->id_ ) );
        if (e != nullptr)
        {
            try {
                writeInstrumentLinks( e, instrument );
            }
            catch (const std::exception& e) {       // xml error
                TRACE( e.what() );
            }
        }
    }


    void Serializer::readInstrumentAttributes( XmlElement* e, Instrument* instrument )
    {
        instrument->id_           = e->getIntAttribute( "id" );
        instrument->presetId_     = e->getIntAttribute( "preset", -1 );
        instrument->name_         = e->getStringAttribute( "name", "Unnamed Instrument" ).toStdString();
        instrument->category_     = e->getStringAttribute( "category" ).toStdString();
        instrument->comment_      = e->getStringAttribute( "comment" ).toStdString();
        instrument->hold_         = e->getBoolAttribute( "hold", instrument->hold_ );
        instrument->retrigger_    = e->getBoolAttribute( "retrigger", instrument->retrigger_ );
        instrument->legato_       = e->getBoolAttribute( "legato", instrument->legato_ );
        instrument->numVoices_    = (uint16_t)e->getIntAttribute( "voices", instrument->numVoices_ );
        instrument->numUnison_    = (uint16_t)e->getIntAttribute( "unison", instrument->numUnison_ );
        instrument->unisonSpread_ = (uint16_t)e->getIntAttribute( "spread", instrument->unisonSpread_ );
    }


    void Serializer::readInstrumentModules( XmlElement* parent, Instrument* instrument )
    {
        XmlElement* modules = getChildElement( parent, "modules" );

        forEachXmlChildElementWithTagName( *modules, e, "module" )
        {
            ModuleType type = (ModuleType)e->getIntAttribute( "type" );
            Module* module = instrument->createAndAddModule( type );

            try {
                module->setId( e->getIntAttribute( "id" ) );
                module->setLabel( e->getStringAttribute( "label", module->getLabel()).toStdString() );
                module->setVoicingType( (VoicingType)e->getIntAttribute( "voicing", module->getVoicingType() ) );
            }
            catch (...) {
                TRACE( "module of type %d could not be created", type );
            }
        }
    }


    void Serializer::readInstrumentLinks( XmlElement* parent, Instrument* instrument )
    {
        XmlElement* links = getChildElement( parent, "links" );

        forEachXmlChildElementWithTagName( *links, e, "link" )
        {
            Link link;
            link.setId( e->getIntAttribute( "id" ));
            link.leftModule_  = e->getIntAttribute( "left_module" );
            link.rightModule_ = e->getIntAttribute( "right_module" );
            link.leftPort_    = e->getIntAttribute( "left_port" );
            link.rightPort_   = e->getIntAttribute( "right_port" );

            instrument->addLink( link, false );
        }
    }


    void Serializer::readInstrumentPresets( XmlElement* parent, Instrument* instrument )
    {
        XmlElement* presetsXml = getChildElement( parent, "presets" );

        forEachXmlChildElementWithTagName( *presetsXml, presetXml, "preset" )
        {
            int id                         = presetXml->getIntAttribute( "id" );
            std::string name               = presetXml->getStringAttribute( "name" ).toStdString();
            const Preset& preset           = instrument->addPreset( id, name );
            ParameterSet& moduleParameters = preset.getModuleParameters();
            ParameterSet& linkParameters   = preset.getLinkParameters();

            forEachXmlChildElementWithTagName( *presetXml, paramXml, "param" )
            {
                int paramId  = paramXml->getIntAttribute( "id", -1 );
                int moduleId = paramXml->getIntAttribute( "module", -1 );
                int linkId   = paramXml->getIntAttribute( "link", -1 );

                if (moduleId >= 0 )
                {
                    Module* module = instrument->getModule( moduleId );
                    ASSERT( module );
                    if (module) 
                    {
                        const Parameter& param = module->getDefaultParameter( paramId );
                        readParameter( paramXml, param );
                        moduleParameters.add( param );
                    }
                }
                else if (linkId >= 0) 
                {
                    const LinkSet& links   = instrument->getLinks();
                    const Link& link       = links.get( linkId );
                    moduleId               = link.leftModule_;
                    const Parameter& param = linkParameters.addLinkParameter( linkId, moduleId );
                    
                    readParameter( paramXml, param );
                }
            }
        }
    }


    void Serializer::readParameter( XmlElement* e, const Parameter& p )
    {
        p.value_        = e->getDoubleAttribute( "value", p.value_ );
        p.defaultValue_ = e->getDoubleAttribute( "default", p.defaultValue_ );
        p.veloSens_     = e->getDoubleAttribute( "vsens", p.veloSens_ );
        p.keyTrack_     = e->getDoubleAttribute( "ktrack", p.keyTrack_ );
        p.resolution_   = e->getDoubleAttribute( "res", p.resolution_ );
        p.label_        = e->getStringAttribute( "label", p.label_ ).toStdString();
        p.unit_         = e->getStringAttribute( "unit", p.unit_ ).toStdString();
        p.numberFormat_ = (NumberFormat)e->getIntAttribute( "fmt", p.numberFormat_ );

        p.valueShaper_.setMin( e->getDoubleAttribute( "min", p.valueShaper_.getMin() ) );
        p.valueShaper_.setMax( e->getDoubleAttribute( "max", p.valueShaper_.getMax() ) );
        p.valueShaper_.setNumSteps( (int16_t)e->getIntAttribute( "steps", p.valueShaper_.getNumSteps() ) );
        p.valueShaper_.setFactor( (int16_t)e->getIntAttribute( "factor", p.valueShaper_.getFactor() ) );

        p.midiShaper_.setControllerId( (int16_t)e->getIntAttribute( "cc", p.midiShaper_.getControllerId() ) );
        p.midiShaper_.setControllerMin( e->getDoubleAttribute( "ccmin", p.midiShaper_.getControllerMin() ) );
        p.midiShaper_.setControllerMax( e->getDoubleAttribute( "ccmax", p.midiShaper_.getControllerMax() ) );
        p.midiShaper_.setSoftTakeover( e->getBoolAttribute( "ccsoft", p.midiShaper_.getSoftTakeover() ) );
    }



    //--------------------------------------------------------------------------------
    // Serializer write methods
    //--------------------------------------------------------------------------------

    void Serializer::writeInstrumentAttributes( XmlElement* e, Instrument* instrument )
    {
        e->removeAllAttributes();

        e->setAttribute( "id", instrument->id_ );
        e->setAttribute( "name", instrument->name_ );
        e->setAttribute( "preset", instrument->presetId_ );
        e->setAttribute( "category", instrument->category_ );
        e->setAttribute( "comment", instrument->comment_ );
        e->setAttribute( "voices", instrument->numVoices_ );

        if (instrument->numUnison_ > 1) {
            e->setAttribute( "unison", instrument->numUnison_ );
            e->setAttribute( "spread", instrument->unisonSpread_ );
        }
        if (instrument->hold_)
            e->setAttribute( "hold", instrument->hold_ );
        if (instrument->retrigger_)
            e->setAttribute( "retrigger", instrument->retrigger_ );
        if (instrument->legato_)
            e->setAttribute( "legato", instrument->legato_ );
    }


    void Serializer::writeInstrumentModules( XmlElement* e, Instrument* instrument )
    {
        XmlElement* modules = getAndClearChildElement( e, "modules" );

        const ModuleList& moduleList = instrument->getModules();
        for (ModuleList::const_iterator it = moduleList.begin(); it != moduleList.end(); it++)
        {
            XmlElement* m = modules->createNewChildElement( "module" );
            writeModule( m, *it );
        }
    }


    void Serializer::writeInstrumentLinks( XmlElement* e, Instrument* instrument )
    {
        XmlElement* links = getAndClearChildElement( e, "links" );

        const LinkSet& linkSet = instrument->getLinks();
        for (LinkSet::const_iterator it = linkSet.begin(); it != linkSet.end(); it++)
        {
            XmlElement* l = links->createNewChildElement( "link" );
            writeLink( l, *it );
        }
    }


    void Serializer::writeInstrumentPresets( XmlElement* e, Instrument* instrument )
    {
        XmlElement* presetsXml = getAndClearChildElement( e, "presets" );

        const PresetSet& presetSet = instrument->getPresets();
        for (PresetSet::const_iterator it = presetSet.begin(); it != presetSet.end(); ++it)
        {
            XmlElement* l = presetsXml->createNewChildElement( "preset" );
            writePreset( l, instrument, *it );
        }
    }


    void Serializer::writeModule( XmlElement* e, Module* module )
    {
        e->setAttribute( "id", module->getId() );
        e->setAttribute( "label", module->getLabel() );
        e->setAttribute( "type", module->moduleType_ );
        e->setAttribute( "poly", module->getVoicingType() );
    }


    void Serializer::writePreset( XmlElement* e, Instrument* instrument, const Preset& preset )
    {
        e->setAttribute( "name", preset.getName() );
        e->setAttribute( "id", preset.getId() );

        const ParameterSet& moduleParameters = preset.getModuleParameters();
        for (ParameterSet::const_iterator it = moduleParameters.begin(); it != moduleParameters.end(); it++)
        {
            const Parameter& param = *it;
            ASSERT( param.controlType_ > ControlHidden );

            Module* module = instrument->getModule( param.getModuleId() );
            ASSERT( module );
            if (module != nullptr)
            {
                XmlElement* p = e->createNewChildElement( "param" );
                p->setAttribute( "module", param.getModuleId() );
                p->setAttribute( "id", param.getId() );

                const Parameter& defaultParam = module->getDefaultParameter( param.getId() );
                writeParameter( p, param, defaultParam );
            }
        }

        const ParameterSet& linkParameters = preset.getLinkParameters();
        for (ParameterSet::const_iterator it = linkParameters.begin(); it != linkParameters.end(); it++)
        {
            const Parameter& param = *it;

            XmlElement* p = e->createNewChildElement( "param" );
            p->setAttribute( "link", param.getId() );

            Parameter defaultParam( param.getId(), param.getModuleId() );
            writeParameter( p, param, defaultParam );
        }
    }


    void Serializer::writeLink( XmlElement* e, const Link& link )
    {
        e->setAttribute( "id", link.getId() );
        e->setAttribute( "left_module", link.leftModule_ );
        e->setAttribute( "right_module", link.rightModule_ );
        e->setAttribute( "left_port", link.leftPort_ );
        e->setAttribute( "right_port", link.rightPort_ );
    }


    void Serializer::writeParameter( XmlElement* e, const Parameter& param, const Parameter& defaultParam )
    {
        e->setAttribute( "value", param.value_ );

        if (param.defaultValue_ != defaultParam.defaultValue_)
            e->setAttribute( "default", param.defaultValue_ );
        if (param.veloSens_ != defaultParam.veloSens_)
            e->setAttribute( "vsens", param.veloSens_ );
        if (param.keyTrack_ != defaultParam.keyTrack_)
            e->setAttribute( "ktrack", param.keyTrack_ );
        if (param.resolution_ != defaultParam.resolution_)
            e->setAttribute( "res", param.resolution_ );
        if (param.controlType_ != defaultParam.controlType_)
            e->setAttribute( "control", param.controlType_ );
        if (param.label_ != defaultParam.label_)
            e->setAttribute( "label", param.label_ );
        if (param.unit_ != defaultParam.unit_)
            e->setAttribute( "unit", param.unit_ );
        if (param.numberFormat_ != defaultParam.numberFormat_)
            e->setAttribute( "fmt", param.numberFormat_ );

        const ParameterShaper& value = param.valueShaper_;
        const ParameterShaper& defaultValue = defaultParam.valueShaper_;
        if (value.getMin() != defaultValue.getMin())
            e->setAttribute( "min", value.getMin() );
        if (value.getMax() != defaultValue.getMax())
            e->setAttribute( "max", value.getMax() );
        if (value.getNumSteps() != defaultValue.getNumSteps())
            e->setAttribute( "steps", value.getNumSteps() );
        if (value.getFactor() != defaultValue.getFactor())
            e->setAttribute( "log", value.getFactor() );

        const MidiParameterShaper& midiValue = param.midiShaper_;
        const MidiParameterShaper& defaultMidiValue = defaultParam.midiShaper_;
        if (midiValue.getControllerId() != defaultMidiValue.getControllerId())
            e->setAttribute( "cc", midiValue.getControllerId() );
        if (midiValue.getControllerMin() != defaultMidiValue.getControllerMin())
            e->setAttribute( "ccmin", midiValue.getControllerMin() );
        if (midiValue.getControllerMax() != defaultMidiValue.getControllerMax())
            e->setAttribute( "ccmax", midiValue.getControllerMax() );
        if (midiValue.getSoftTakeover() != defaultMidiValue.getSoftTakeover())
            e->setAttribute( "ccsoft", midiValue.getSoftTakeover() );
    }


    XmlElement* Serializer::getChildElement( XmlElement* e, const std::string& name )
    {
        XmlElement* c = e->getChildByName( StringRef(name) );

        if (c == nullptr) {
            c = e->createNewChildElement( StringRef(name) );
        }
        return c;
    }


    XmlElement* Serializer::getAndClearChildElement( XmlElement* e, const std::string& name )
    {
        XmlElement* c = getChildElement( e, name );
        c->deleteAllChildElements();
        return c;
    }



    File Serializer::checkPath( const std::string& path )
    {
        StringRef p = path.c_str();
        File f = File::isAbsolutePath( p ) ? File( path ) : File::getCurrentWorkingDirectory().getChildFile( p );

        if (f.existsAsFile() == false)  {
            THROW( std::invalid_argument, "bank file not found: %s", path );
        }
        return f;
    }


    void Serializer::checkPath( File& file )
    {
        StringRef path = file.getFullPathName();

        if (File::isAbsolutePath( path ) == false) {
            file = File::getCurrentWorkingDirectory().getChildFile( path );
        }
    }


} // namespace e3