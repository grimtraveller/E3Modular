
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>


namespace e3 {
    class Settings;
    class Instrument;

    class Bank {
        friend class BankSerializer;

    public:
        //void close(bool doStore, bool makeBackup);

        XmlElement* open(const std::string& path);
        XmlElement* createNewBank();
        //void load(const std::string& filepath="", bool append=false);
        void store(const std::string& path, bool saveCurrent=true, bool makeBackup=true);


        const std::string getName() const;    
        void setName(const std::string& name); 
                                                     
        int getInstrumentHash() const;
        void setInstrumentHash(int hash);

        void setPath(const std::string& path);
        std::string getPath();

        Instrument* loadInstrument(int hash=0);
        void storeInstrument(Instrument* instrument);

        void append(Instrument* instrument);

        void setSettings(Settings* settings);

        XmlElement* getXmlRoot();
        void setXmlRoot(XmlElement* newRoot);

    protected:
        Settings* settings_ = nullptr;
        bool autosave_      = true;
        bool makeBackup_    = false;

        std::unique_ptr<XmlElement> xmlRoot_ = nullptr;
        std::unique_ptr<XmlDocument> xmlDocument_ = nullptr;
    };
}  // namespace e3
