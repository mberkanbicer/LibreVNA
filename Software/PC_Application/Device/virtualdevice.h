#ifndef VIRTUALDEVICE_H
#define VIRTUALDEVICE_H

#include "device.h"
#include "Tools/parameters.h"
#include "compounddevice.h"

#include <set>
#include <complex>

#include <QObject>

class VirtualDevice : public QObject
{
    Q_OBJECT
public:
    VirtualDevice(QString serial = "");
    ~VirtualDevice();

    class Info {
    public:
        uint16_t ProtocolVersion;
        uint8_t FW_major;
        uint8_t FW_minor;
        uint8_t FW_patch;
        uint8_t hardware_version;
        char HW_Revision;
        int ports;
        bool supportsVNAmode;
        bool supportsSAmode;
        bool supportsSGmode;
        bool supportsExtRef;
        struct {
            double minFreq, maxFreq, maxFreqHarmonic;
            double minIFBW, maxIFBW;
            unsigned int maxPoints;
            double mindBm;
            double maxdBm;
            double minRBW, maxRBW;
        } Limits;
    };

    class Status {
    public:
        QString statusString;
        bool overload;
        bool unlocked;
        bool unlevel;
        bool extRef;
    };

    static void RegisterTypes();

    bool isCompoundDevice() const;
    Device *getDevice();
    CompoundDevice *getCompoundDevice();
    std::vector<Device*> getDevices();
    const Info& getInfo() const;
    static const VirtualDevice::Info &getInfo(VirtualDevice *vdev);
    const Status &getStatus() const;
    static const VirtualDevice::Status &getStatus(VirtualDevice *vdev);

    class VNASettings {
    public:
        double freqStart, freqStop;
        double dBmStart, dBmStop;
        double IFBW;
        int points;
        bool logSweep;
        std::vector<int> excitedPorts; // port count starts at one
    };
    class VNAMeasurement {
    public:
        unsigned int pointNum;
        double Z0;
        union {
            struct {
                // for non-zero span
                double frequency;
                double dBm;
            };
            struct {
                // for zero span
                double us; // time in us since first datapoint
            };
        };
        std::map<QString, std::complex<double>> measurements;

        Sparam toSparam(int port1, int port2);
        void fromSparam(Sparam S, int port1, int port2);
        VNAMeasurement interpolateTo(const VNAMeasurement &to, double a);
    };

    QStringList availableVNAMeasurements();
    bool setVNA(const VNASettings &s, std::function<void(bool)> cb = nullptr);
    QString serial();

    class SASettings {
    public:
        enum class Window {
            None = 0,
            Kaiser = 1,
            Hann = 2,
            FlatTop = 3,
            Last
        };
        enum class Detector {
            PPeak = 0,
            NPeak = 1,
            Sample = 2,
            Normal = 3,
            Average = 4,
            Last
        };

        double freqStart, freqStop;
        double RBW;
        int points;
        Window window;
        Detector detector;
        bool signalID;
        bool trackingGenerator;
        int trackingPort;
        double trackingOffset;
        double trackingPower;
    };
    class SAMeasurement {
    public:
        int pointNum;
        union {
            struct {
                // for non-zero span
                double frequency;
                double cdbm;
            };
            struct {
                // for zero span
                double us; // time in us since first datapoint
            };
        };
        std::map<QString, double> measurements;
    };

    QStringList availableSAMeasurements();
    bool setSA(const SASettings &s, std::function<void(bool)> cb = nullptr);

    class SGSettings {
    public:
        double freq;
        double dBm;
        int port;
    };

    QStringList availableSGPorts();
    bool setSG(const SGSettings &s);

    bool setIdle(std::function<void(bool)> cb = nullptr);

    QStringList availableExtRefInSettings();
    QStringList availableExtRefOutSettings();

    bool setExtRef(QString option_in, QString option_out);

    static std::set<QString> GetDevices();
    static VirtualDevice* getConnected();

signals:
    void VNAmeasurementReceived(VNAMeasurement m);
    void SAmeasurementReceived(SAMeasurement m);
    void ConnectionLost();
    void InfoUpdated();
    void StatusUpdated(Status status);
    void LogLineReceived(QString line);
    void NeedsFirmwareUpdate(int usedProtocol, int requiredProtocol);
private:
    Info info;
    Status status;
    bool isCompound;
    std::vector<Device*> devices;
    bool zerospan;

    std::map<Device*, Device::TransmissionResult> results;

    CompoundDevice *cdev;

    std::map<int, std::vector<Protocol::VNADatapoint<32>*>> compoundDataBuffer;

    std::map<int, int> portStageMapping; // maps from excitedPort (count starts at zero) to stage (count starts at zero)
};

Q_DECLARE_METATYPE(VirtualDevice::Status)
Q_DECLARE_METATYPE(VirtualDevice::VNAMeasurement)
Q_DECLARE_METATYPE(VirtualDevice::SAMeasurement)

#endif // VIRTUALDEVICE_H