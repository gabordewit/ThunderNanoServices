#pragma once

#include "Module.h"
#include <interfaces/IBluetooth.h>
#include <interfaces/IKeyHandler.h>

namespace WPEFramework {

namespace Plugin {

class BluetoothControl : public PluginHost::IPlugin
                       , public PluginHost::IWeb
                       , public Exchange::IBluetooth
                       , public Exchange::IKeyHandler {
    private:
        class ManagementFlow {
        public:
            ManagementFlow() = delete;
            ManagementFlow(const ManagementFlow&) = delete;
            ManagementFlow& operator=(const ManagementFlow&) = delete;
            ManagementFlow(const TCHAR formatter[], ...)
            {
                va_list ap;
                va_start(ap, formatter);
                Trace::Format(_text, formatter, ap);
                va_end(ap);
            }
            explicit ManagementFlow(const string& text)
                : _text(Core::ToString(text))
            {
            }
            ~ManagementFlow()
            {
            }

        public:
            inline const char* Data() const
            {
                return (_text.c_str());
            }
            inline uint16_t Length() const
            {
                return (static_cast<uint16_t>(_text.length()));
            }

        private:
            std::string _text;
        };

        class ControlFlow {
        public:
            ControlFlow() = delete;
            ControlFlow(const ControlFlow&) = delete;
            ControlFlow& operator=(const ControlFlow&) = delete;
            ControlFlow(const TCHAR formatter[], ...)
            {
                va_list ap;
                va_start(ap, formatter);
                Trace::Format(_text, formatter, ap);
                va_end(ap);
            }
            explicit ControlFlow(const string& text)
                : _text(Core::ToString(text))
            {
            }
            ~ControlFlow()
            {
            }

        public:
            inline const char* Data() const
            {
                return (_text.c_str());
            }
            inline uint16_t Length() const
            {
                return (static_cast<uint16_t>(_text.length()));
            }

        private:
            std::string _text;
        };

        class DeviceFlow {
        public:
            DeviceFlow() = delete;
            DeviceFlow(const DeviceFlow&) = delete;
            DeviceFlow& operator=(const DeviceFlow&) = delete;
            DeviceFlow(const TCHAR formatter[], ...)
            {
                va_list ap;
                va_start(ap, formatter);
                Trace::Format(_text, formatter, ap);
                va_end(ap);
            }
            explicit DeviceFlow(const string& text)
                : _text(Core::ToString(text))
            {
            }
            ~DeviceFlow()
            {
            }

        public:
            inline const char* Data() const
            {
                return (_text.c_str());
            }
            inline uint16_t Length() const
            {
                return (static_cast<uint16_t>(_text.length()));
            }

        private:
            std::string _text;
        };

        class ControlSocket : public Bluetooth::HCISocket {
        private:
            class ManagementSocket : public Bluetooth::ManagementSocket {
            public:
                ManagementSocket(const ManagementSocket&) = delete;
                ManagementSocket& operator= (const ManagementSocket&) = delete;

                ManagementSocket(ControlSocket& parent)
                    : Bluetooth::ManagementSocket()
                    , _parent(parent)
                {
                }
                ~ManagementSocket()
                {
                }

            private:
                void Update(const mgmt_hdr& header) override
                {
                    const uint8_t* data = &(reinterpret_cast<const uint8_t*>(&header)[sizeof(mgmt_hdr)]);
                    uint16_t opCode = btohs(header.opcode);
                    uint16_t device = btohs(header.index);
                    uint16_t packageLen = btohs(header.len);

                    switch (opCode) {
                    case MGMT_EV_CONTROLLER_ERROR: {
                        const mgmt_ev_controller_error* info = reinterpret_cast<const mgmt_ev_controller_error*>(data);
                        TRACE(ManagementFlow, (_T("MGMT_EV_CONTROLLER_ERROR [%d]"), info->error_code));
                        break;
                    }
                    case MGMT_EV_NEW_CONN_PARAM: {
                        const mgmt_ev_new_conn_param* info = reinterpret_cast<const mgmt_ev_new_conn_param*>(data);
                        TRACE(ManagementFlow, (_T("MGMT_EV_NEW_CONN_PARAM timeout [%d]"), info->timeout));
                        break;
                    }
                    case MGMT_EV_DEVICE_CONNECTED: {
                        const mgmt_ev_device_connected* info = reinterpret_cast<const mgmt_ev_device_connected*>(data);
                        TRACE(ManagementFlow, (_T("MGMT_EV_DEVICE_CONNECTED device [%s], flags [%02X]"), Bluetooth::Address(info->addr.bdaddr).ToString().c_str(), info->flags));
                        break;
                    }
                    case MGMT_EV_DEVICE_DISCONNECTED: {
                        const mgmt_ev_device_disconnected* info = reinterpret_cast<const mgmt_ev_device_disconnected*>(data);
                        TRACE(ManagementFlow, (_T("MGMT_EV_DEVICE_DISCONNECTED device [%s], reason:%d"), Bluetooth::Address(info->addr.bdaddr).ToString().c_str(), info->reason));
                        break;
                    }
                    case MGMT_EV_NEW_IRK: {
                        const mgmt_ev_new_irk* info = reinterpret_cast<const mgmt_ev_new_irk*>(data);
                        string key;
                        Core::ToHexString(info->key.val, 16, key);
                        TRACE(ManagementFlow, (_T("MGMT_EV_NEW_IRK")));
                        TRACE(ManagementFlow, (_T("  store_hint=%d, rpa=%s"), info->store_hint, Bluetooth::Address(info->rpa).ToString().c_str()));
                        TRACE(ManagementFlow, (_T("  key.addr=%s, key.addr.type=%i, key.val=%s"), Bluetooth::Address(info->key.addr.bdaddr).ToString().c_str(), info->key.addr.type, key.c_str()));
                        if (info->store_hint != 0) {
                            if (_parent.NewKey(Bluetooth::IdentityKey(info->key.addr.bdaddr, info->key.addr.type, info->key.val)) != Core::ERROR_NONE) {
                                TRACE(Trace::Error, (_T("Invalid IRK received?")));
                            }
                        }
                        break;
                    }
                    case MGMT_EV_NEW_CSRK: {
                        const mgmt_ev_new_csrk* info = reinterpret_cast<const mgmt_ev_new_csrk*>(data);
                        string key;
                        Core::ToHexString(info->key.val, 16, key);
                        TRACE(ManagementFlow, (_T("MGMT_EV_NEW_CSRK")));
                        TRACE(ManagementFlow, (_T("  store_hint=%d"), info->store_hint));
                        TRACE(ManagementFlow, (_T("  key.addr=%s, key.addr.type=%i, key.type=%d, key.val=%s"), Bluetooth::Address(info->key.addr.bdaddr).ToString().c_str(), info->key.addr.type, info->key.type, key.c_str()));
                        if (info->store_hint != 0) {
                            if (_parent.NewKey(Bluetooth::SignatureKey(info->key.addr.bdaddr, info->key.addr.type, info->key.type, info->key.val)) != Core::ERROR_NONE) {
                                TRACE(Trace::Error, (_T("Invalid CSRK received?")));
                            }
                        }
                        break;
                    }
                    case MGMT_EV_NEW_LONG_TERM_KEY: {
                        const mgmt_ev_new_long_term_key* info = reinterpret_cast<const mgmt_ev_new_long_term_key*>(data);
                        string key;
                        Core::ToHexString(info->key.val, 16, key);
                        TRACE(ManagementFlow, (_T("MGMT_EV_NEW_LONG_TERM_KEY")));
                        TRACE(ManagementFlow, (_T("  store_hint=%d"), info->store_hint));
                        TRACE(ManagementFlow, (_T("  key.addr=%s, key.addr.type=%i, key.type=%d, key.master=%d"), Bluetooth::Address(info->key.addr.bdaddr).ToString().c_str(), info->key.addr.type, info->key.type, info->key.master));
                        TRACE(ManagementFlow, (_T("  key.enc_size=%u, key.ediv=%u, key.rand=%llu, key.val=%s"), info->key.enc_size, btohs(info->key.ediv), btohll(info->key.rand), key.c_str()));

                        if (info->store_hint != 0) {
                            if (_parent.NewKey(Bluetooth::LongTermKey(info->key.addr.bdaddr, info->key.addr.type, info->key.type, info->key.master,
                                                                  info->key.enc_size, btohs(info->key.ediv), btohll(info->key.rand), info->key.val)) == Core::ERROR_NONE) {

                                _parent.Paired(Bluetooth::Address(info->key.addr.bdaddr));
                            } else {
                                TRACE(Trace::Error, (_T("Invalid LTK received?")));
                            }
                        }

                        break;
                    }
                    default:
                        TRACE(ManagementFlow, (_T("Device=%d, OpCode=0x%04X, Length=%d"), device, opCode, packageLen));
                        if (packageLen > 0) {
                            string dataText;
                            Core::ToHexString(data, packageLen, dataText);
                            TRACE(ManagementFlow, (_T("Data=%s"), dataText.c_str()));
                        }
                        break;
                    }
                }

            private:
                ControlSocket& _parent;
            };

            // The bluetooth library has some unexpected behaviour. For example, the scan of NON-BLE devices
            // is a blocking call for the duration of the passed in time. Which is, I think, very intrusive
            // fo any responsive design. If a RESTFull call would start a scan, the call would last the duration
            // of the scan, which is typicall >= 10Secods which is unacceptable, so it needs to be decoupled.
            // This decoupling is done on this internal Worker thread.
            class Job : public Core::IDispatch {
            private:
                Job() = delete;
                Job(const Job&) = delete;
                Job& operator=(const Job&) = delete;

                enum scanMode {
                    LOW_ENERGY = 0x01,
                    REGULAR = 0x02,
                    PASSIVE = 0x04,
                    LIMITED = 0x08
                };

            public:
                Job(ControlSocket* parent)
                    : _parent(*parent)
                    , _mode(0)
                {
                }
                virtual ~Job()
                {
                }

            public:
                void Load(const uint16_t scanTime, const uint32_t type, const uint8_t flags)
                {
                    if (_mode == 0) {
                        _mode = REGULAR;
                        _scanTime = scanTime;
                        _type = type;
                        _flags = flags;
                        PluginHost::WorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(*this));
                    }
                }
                void Load(const uint16_t scanTime, const bool limited, const bool passive)
                {
                    if (_mode == 0) {
                        _mode = LOW_ENERGY | (passive ? PASSIVE : 0) | (limited ? LIMITED : 0);
                        _scanTime = scanTime;
                        PluginHost::WorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(*this));
                    }
                }

            private:
                virtual void Dispatch()
                {
                    if ((_mode & REGULAR) != 0) {
                        TRACE(ControlFlow, (_T("Start regular scan: %s"), Core::Time::Now().ToRFC1123().c_str()));
                        _parent.Run(_scanTime, _type, _flags);
                    } else {
                        TRACE(ControlFlow, (_T("Start Low Energy scan: %s"), Core::Time::Now().ToRFC1123().c_str()));
                        _parent.Run(_scanTime, ((_mode & LIMITED) != 0), ((_mode & PASSIVE) != 0));
                    }
                    TRACE(ControlFlow, (_T("Scan completed: %s"), Core::Time::Now().ToRFC1123().c_str()));
                    _mode = 0;
                }

            private:
                ControlSocket& _parent;
                uint16_t _scanTime;
                uint32_t _type;
                uint8_t _flags;
                uint8_t _mode;
            };

        public:
            ControlSocket(const ControlSocket&) = delete;
            ControlSocket& operator=(const ControlSocket&) = delete;

            ControlSocket()
                : Bluetooth::HCISocket()
                , _parent(nullptr)
                , _activity(Core::ProxyType<Job>::Create(this))
                , _administrator(*this)
            {
            }
            virtual ~ControlSocket()
            {
            }

        public:
            Bluetooth::ManagementSocket& Control()
            {
                return(_administrator);
            }
            uint32_t Pair(const Bluetooth::Address& remote, const Bluetooth::Address::type type, const Bluetooth::ManagementSocket::capabilities caps)
            {
                return(_administrator.Pair(remote, type, caps));
            }
            uint32_t Unpair(const Bluetooth::Address& remote, const Bluetooth::Address::type type)
            {
                if (_parent != nullptr) {
                    _parent->PurgeDeviceKeys(remote);
                }

                return(_administrator.Unpair(remote, type));
            }
            void Paired(const Bluetooth::Address& remote)
            {
                if (_parent != nullptr) {
                    DeviceImpl* entry = _parent->Find(remote);
                    if (entry != nullptr) {
                        if (entry->IsPaired() == false) {
                            entry->Paired(true);
                        } else {
                            // have both LTKs
                            if (_parent->SaveLeEncryptionKeys() == Core::ERROR_NONE) {
                                entry->Bonded(true);
                            }
                        }
                    }
                }
            }
            void Scan(const uint16_t scanTime, const uint32_t type, const uint8_t flags)
            {
                if (IsOpen() == true) {
                    _activity->Load(scanTime, type, flags);
                }
            }
            void Scan(const uint16_t scanTime, const bool limited, const bool passive)
            {
                if (IsOpen() == true) {
                    _activity->Load(scanTime, limited, passive);
                }
            }
            uint32_t Open(BluetoothControl& parent)
            {
                ASSERT (IsOpen() == false);

                _parent = &parent;

                Bluetooth::HCISocket::LocalNode(Core::NodeId(_administrator.DeviceId(), HCI_CHANNEL_RAW));
                return (Bluetooth::HCISocket::Open(Core::infinite));
            }
            uint32_t Close()
            {
                uint32_t result = Bluetooth::HCISocket::Close(Core::infinite);
                PluginHost::WorkerPool::Instance().Revoke(Core::ProxyType<Core::IDispatch>(_activity));
                Bluetooth::ManagementSocket::Down(_administrator.DeviceId());
                _administrator.DeviceId(HCI_DEV_NONE);
                _parent = nullptr;
                return (result);
            }
            template<typename KEYTYPE> uint32_t NewKey(const KEYTYPE& key)
            {
                uint32_t result = Core::ERROR_UNAVAILABLE;

                if (_parent != nullptr) {
                    if (key.IsValid() == true) {
                        _parent->StoreKey(key);
                        result = Core::ERROR_NONE;
                    } else {
                        result = Core::ERROR_INVALID_DESIGNATOR;
                    }
                }

                return result;
            }
            void PurgeDeviceKeys(const Bluetooth::Address& address)
            {
                if (_parent != nullptr) {
                    _parent->PurgeDeviceKeys(address);
                }
            }

        private:
            void Discovered(const bool lowEnergy, const Bluetooth::Address& address, const string& name) override
            {
                if (_parent != nullptr) {
                    _parent->Discovered(lowEnergy, address, name);
                }
            }
            void Update(const le_advertising_info& eventData) override
            {
                string name;
                uint8_t type = Name (eventData, name);

                if ( (type != 0) && (_parent != nullptr) ) {
                    _parent->Discovered(true, Bluetooth::Address(eventData.bdaddr), name);
                }
            }
            void Run(const uint16_t scanTime, const uint32_t type, const uint8_t flags)
            {
                Bluetooth::HCISocket::Scan(scanTime, type, flags);
            }
            void Run(const uint16_t scanTime, const bool limited, const bool passive)
            {
                Bluetooth::HCISocket::Scan(scanTime, limited, passive);
            }
            void Update(const hci_event_hdr& eventData) override {
                const uint8_t* data = &(reinterpret_cast<const uint8_t*>(&eventData)[sizeof(hci_event_hdr)]);

                switch (eventData.evt) {
                    case EVT_VENDOR: {
                        TRACE(ControlFlow, (_T("EVT_VENDOR vendor specific information, length [%d]"), eventData.plen));
                        break;
                    }
                    case EVT_CMD_STATUS: {
                        const evt_cmd_status* cs = reinterpret_cast<const evt_cmd_status*>(data);
                        uint16_t opcode = htobs(cs->opcode);
                        uint8_t cat = (opcode >> 10) & 0x3F;
                        uint16_t id = (opcode & 0x3FF);
                        TRACE(ControlFlow, (_T("EVT_CMD_STATUS OpCode: %02X:%03X, Status: %d"), cat, id, cs->status));
                        break;
                    }
                    case EVT_CMD_COMPLETE: {
                        const evt_cmd_complete* cc = reinterpret_cast<const evt_cmd_complete*>(data);
                        uint16_t opcode = htobs(cc->opcode);
                        uint8_t cat = (opcode >> 10) & 0x3F;
                        uint16_t id = (opcode & 0x3FF);
                        TRACE(ControlFlow, (_T("EVT_CMD_COMPLETE OpCode: %02X:%03X"), cat, id));
                        break;
                    }
                    case EVT_DISCONN_COMPLETE: {
                        const evt_disconn_complete* info = reinterpret_cast<const evt_disconn_complete*>(data);
                        TRACE(ControlFlow, (_T("EVT_DISCONN_COMPLETE (status:%d, handle:%d, reason:%d)"), info->status, info->handle, info->reason));
                        if ((info->status == 0) && (_parent != nullptr)) {
                            DeviceImpl* entry = _parent->Find(info->handle);
                            if (entry != nullptr) {
                                entry->Disconnection(info->reason);
                            }
                        }
                        break;
                    }
                    case EVT_LE_META_EVENT: {
                        const evt_le_meta_event* input = reinterpret_cast<const evt_le_meta_event*>(data);

                        if (input->subevent == EVT_LE_CONN_COMPLETE) {
                            const evt_le_connection_complete* info = reinterpret_cast<const evt_le_connection_complete*>(input->data);
                            Bluetooth::Address host (info->peer_bdaddr);
                            TRACE(ControlFlow, (_T("EVT_LE_CONN_COMPLETE (status:%d, handle:%d, bdaddr:%s)"), info->status, info->handle,
                                Bluetooth::Address(info->peer_bdaddr).ToString().c_str()));

                            if ( (info->status == 0) && (_parent != nullptr) )  {
                                DeviceImpl* entry = _parent->Find(host);
                                if (entry != nullptr) {
                                    entry->Connection(info->handle, info->role);
                                }
                            }
                        } else if (input->subevent == EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE) {
                            const evt_le_read_remote_used_features_complete* info = reinterpret_cast<const evt_le_read_remote_used_features_complete*>(input->data);
                            TRACE(ControlFlow, (_T("EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE (status:%d, handle:%d)"), info->status, info->handle));

                            if ( (info->status == 0) && (_parent != nullptr) ) {
                                DeviceImpl* entry = _parent->Find(info->handle);
                                if (entry != nullptr) {
                                    entry->Features(sizeof(info->features), info->features);
                                }
                            }
                        } else if (input->subevent == EVT_LE_CONN_UPDATE_COMPLETE) {
                            const evt_le_connection_update_complete* info = reinterpret_cast<const evt_le_connection_update_complete*>(input->data);
                            TRACE(ControlFlow, (_T("EVT_LE_CONN_UPDATE_COMPLETE (status:%d, handle:%d)"), info->status, info->handle));

                            if ( (info->status == 0)  && (_parent != nullptr) ) {
                                DeviceImpl* entry = _parent->Find(info->handle);
                                if (entry != nullptr) {
                                }
                            }
                        } else {
                            TRACE(ControlFlow, (_T("EVT_LE_META_EVENT: unexpected subevent: %d"), input->subevent));
                        }
                        break;
                    }
                    case EVT_IO_CAPABILITY_RESPONSE: {
                        const evt_io_capability_response* info = reinterpret_cast<const evt_io_capability_response*>(data);
                        Bluetooth::Address host (info->bdaddr);
                        TRACE(ControlFlow, (_T("EVT_IO_CAPABILITY_RESPONSE: %s"), host.ToString().c_str()));
                        if (_parent != nullptr) {
                            _parent->Capabilities(host, info->capability, info->authentication, info->oob_data);
                        }
                        break;
                    }
                    default:
                         TRACE(ControlFlow, (_T("UNKNOWN_EVENT: %x"), eventData.evt));
                         break;
                }
            }

        private:
            BluetoothControl* _parent;
            Core::ProxyType<Job> _activity;
            ManagementSocket _administrator;
        };

        class GATTRemote : public Bluetooth::GATTSocket, public Exchange::IKeyProducer {
        private:
            // UUID
            static constexpr uint16_t HID_UUID         = 0x1812;
            static constexpr uint16_t REPORT_UUID      = 0x2a4d;
/*
            enum state : uint8_t {
                UNKNOWN     = 0x00,
                PAIRING     = 0x01,
                UNPAIRING   = 0x02,
                OPERATIONAL = 0x04
            };
*/

            class Flow {
            public:
                Flow() = delete;
                Flow(const Flow&) = delete;
                Flow& operator=(const Flow&) = delete;
                Flow(const TCHAR formatter[], ...)
                {
                    va_list ap;
                    va_start(ap, formatter);
                    Trace::Format(_text, formatter, ap);
                    va_end(ap);
                }
                explicit Flow(const string& text)
                    : _text(Core::ToString(text))
                {
                }
                ~Flow()
                {
                }

            public:
                inline const char* Data() const
                {
                    return (_text.c_str());
                }
                inline uint16_t Length() const
                {
                    return (static_cast<uint16_t>(_text.length()));
                }

            private:
                std::string _text;
            };

            class Sink : public Exchange::IBluetooth::IDevice::ICallback {
            public:
                Sink() = delete;
                Sink(const Sink&) = delete;
                Sink& operator=(const Sink&) = delete;

                Sink(GATTRemote* parent) : _parent(*parent)
                {
                    ASSERT(parent != nullptr);
                }
                virtual ~Sink()
                {
                }

            public:
                void Updated() override
                {
                    _parent.Updated();
                }

                BEGIN_INTERFACE_MAP(Sink)
                    INTERFACE_ENTRY(IBluetooth::IDevice::ICallback)
                END_INTERFACE_MAP

            private:
                GATTRemote& _parent;
            };

            class Activity : public Core::IDispatch {
            public:
                Activity(const Activity &) = delete;
                Activity &operator=(const Activity &) = delete;
                Activity()
                    : _pressed(false)
                    , _scancode(0)
                    , _map()
                    , _keyHandler(nullptr)
                    , _adminLock()
                {
                }
                ~Activity() override
                {
                }

                void KeyEvent(bool pressed, uint16_t scancode, const string& map)
                {
                    _adminLock.Lock();
                    if ((scancode != 0) && (_keyHandler != nullptr)) {
                        _pressed = pressed;
                        _scancode = scancode;
                        _map = map;
                        PluginHost::WorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(*this));
                    }
                    _adminLock.Unlock();
                }
                uint32_t KeyHandler(IKeyHandler* keyHandler)
                {
                    _adminLock.Lock();
                    if (_keyHandler != nullptr) {
                        _keyHandler->Release();
                    }
                    if (keyHandler != nullptr) {
                        keyHandler->AddRef();
                    }
                    _keyHandler = keyHandler;
                    _adminLock.Unlock();
                    return (Core::ERROR_NONE);
                }
                void Dispatch() override
                {
                    _adminLock.Lock();
                    if (_keyHandler != nullptr) {
                        _keyHandler->KeyEvent(_pressed, _scancode, _map);
                    }
                    _adminLock.Unlock();
                }

            private:
                bool _pressed;
                uint16_t _scancode;
                string _map;
                IKeyHandler* _keyHandler;
                Core::CriticalSection _adminLock;
            };

            static Core::NodeId Designator(const uint8_t type, const string& address)
            {
                return(Bluetooth::Address(address.c_str()).NodeId(static_cast<Bluetooth::Address::type>(type), GATTSocket::LE_ATT_CID, 0));
            }

        public:
            GATTRemote() = delete;
            GATTRemote(const GATTRemote&) = delete;
            GATTRemote& operator=(const GATTRemote&) = delete;

            GATTRemote(BluetoothControl* parent, IBluetooth::IDevice* device)
                : Bluetooth::GATTSocket(
                      Designator(device->Type(), device->LocalId()),
                      Designator(device->Type(), device->RemoteId()),
                      64)
                , _adminLock()
//                , _state(UNKNOWN)
                , _profile(true)
                , _command()
                , _device(device)
                , _handles()
                , _sink(this)
                , _currentKey(0)
                , _activity(Core::ProxyType<Activity>::Create())
                , _parent(parent)
            {
                ASSERT(_parent != nullptr);

                _device->AddRef();
                _name = _device->Name();

                if (_device->Callback(&_sink) != Core::ERROR_NONE) {
                    TRACE(Trace::Fatal, (_T("The device is already in use. Only 1 callback allowed")));
                }
                else if (_device->IsConnected() == true) {
                    TRACE(Trace::Fatal, (_T("The device is already connected. First disconnect the device")));
                }
            }
            virtual ~GATTRemote()
            {
                if (GATTSocket::IsOpen() == true) {
                    GATTSocket::Close(Core::infinite);
                }
                if (_device != nullptr) {
                    if (_device->Callback(nullptr) != Core::ERROR_NONE) {
                        TRACE(Trace::Fatal, (_T("Could not remove the callback from the device. Scarry !!!")));
                    }
                    _device->Release();
                    _device = nullptr;
                }
            }

            BEGIN_INTERFACE_MAP(BluetoothRemoteControl)
            INTERFACE_ENTRY(Exchange::IKeyProducer)
            END_INTERFACE_MAP

        public:
            // IKeyProducer methods
            const TCHAR* Name() const override
            {
                return (_name.c_str());
            }
            bool Pair() override
            {
                return (Connect() == Core::ERROR_NONE);
            }
            bool Unpair(string bondingId) override
            {
                return (Disconnect() == Core::ERROR_NONE);
            }
            uint32_t Callback(IKeyHandler* callback) override
            {
                return (_activity->KeyHandler(callback));
            }
            uint32_t Error() const override
            {
                return 0;
            }
            string MetaData() const override
            {
                return {};
            }
            void Configure(const string& settings) override
            {
            }

/*
            uint32_t Pair()
            {
                uint32_t result = false;

                _adminLock.Lock();

                if (_device != nullptr) {
                    result = Core::ERROR_INPROGRESS;
                    if ( (_state & (PAIRING|UNPAIRING)) == 0) {
                        IBluetooth::IDevice* device = _device;
                        device->AddRef();
                        _state = static_cast<state>(_state | PAIRING);
                        _adminLock.Unlock();

                        result = device->Pair(IBluetooth::IDevice::DISPLAY_ONLY);
                        device->Release();

                        _adminLock.Lock();
                        // _state = static_cast<state>(_state & (~PAIRING));
                    }
                }

                _adminLock.Unlock();

                return (result);
            }
            uint32_t Unpair()
            {
                uint32_t result = false;

                _adminLock.Lock();

                if (_device != nullptr) {
                    result = Core::ERROR_INPROGRESS;
                    if ( (_state & (PAIRING|UNPAIRING)) == 0) {
                        IBluetooth::IDevice* device = _device;
                        device->AddRef();
                        _state = static_cast<state>(_state | UNPAIRING);
                        _adminLock.Unlock();

                        result = device->Unpair();
                        device->Release();

                        _adminLock.Lock();
                        _state = static_cast<state>(_state & (~UNPAIRING));
		            }
                }

                _adminLock.Unlock();

                return (result);
            }
*/

            uint32_t Connect()
            {
                uint32_t result = Core::ERROR_NONE;

                if ((IsOpen() == false) && (IsConnecting() == false)) {
                    TRACE(Trace::Information, (_T("Connecting GATT socket %s"), _device->RemoteId().c_str()));
                    result = GATTSocket::Open(5000);
                    if ((result != Core::ERROR_NONE) && (result != Core::ERROR_INPROGRESS)) {
                        TRACE(Flow, (_T("Opening GATT socket [%s], failed: %i"), _device->RemoteId().c_str(), result));
                    }
                } else {
                    TRACE(Flow, (_T("GATT socket already open")));
                }

                return result;
            }
            uint32_t Disconnect()
            {
                uint32_t result = Core::ERROR_NONE;

                if (IsOpen() == true) {
                    TRACE(Trace::Information, (_T("Disconnecting GATT socket %s"), _device->RemoteId().c_str()));
                    result = GATTSocket::Close(Core::infinite);
                } else {
                    TRACE(Flow, (_T("GATT socket already closed")));
                }

                return (result);
            }

        private:
            bool Initialize() override
            {
                return (Security(BT_SECURITY_LOW));
            }
            void Notification(const uint16_t handle, const uint8_t dataFrame[], const uint16_t length) override
            {
                if ((handle == 0x34) && (length >= 2)) {
                    uint16_t scancode = ((dataFrame[1] << 8) | dataFrame[0]);
                    bool pressed = (scancode != 0);

                    if (pressed == true) {
                        _currentKey = scancode;
                    }

                    if (_currentKey != 0) {
                        TRACE(Flow, (_T("Received a keypress notification: handle=0x%x, code=%i, state=%s"), handle, _currentKey, (pressed? "pressed" : "released")));
                        ASSERT(_activity != nullptr);
                        _activity->KeyEvent(pressed, _currentKey, _name);
                    }
                } else {
                    string data;
                    Core::ToHexString(dataFrame, length, data);
                    printf(_T("Received an unknown notification: [handle=0x%x], %d bytes: %s\n"), handle, length, data.c_str());
                }
            }
            void Operational() override
            {
                TRACE(Flow, (_T("The received MTU: %d"), MTU()));

                _profile.Discover(CommunicationTimeOut * 20, *this, [&](const uint32_t result) {
                    if (result == Core::ERROR_NONE) {
                        if (_profile[Bluetooth::UUID(HID_UUID)] == nullptr) {
                            TRACE(Flow, (_T("The given bluetooth device does not support a HID service!!")));
                        }
                        else {
                            LoadReportHandles();

                            if (_handles.size() > 0) {
                                uint16_t val = htobs(1);
                                _command.Write(_handles.front(), sizeof(val), reinterpret_cast<const uint8_t*>(&val));
                                Execute(CommunicationTimeOut, _command, [&](const GATTSocket::Command& cmd) {
                                    EnableEvents(cmd);
                                });

                                _parent->RemoteControlConnected(*this);
                            }
                        }
                    }
                    else {
//                        _state = UNKNOWN;
                        TRACE(Flow, (_T("The given bluetooth device could not be read for services!!")));
                    }
                });
            }
            void LoadReportHandles()
            {
                const Bluetooth::UUID AUDIO_SERVICE_UUID(_T("f0e0d000-a000-b000-c000-987654321000"));
                const Bluetooth::UUID AUDIO_COMMAND_UUID(_T("f0e0d001-a000-b000-c000-987654321000"));
                const Bluetooth::UUID AUDIO_DATA_UUID(_T("f0e0d002-a000-b000-c000-987654321000"));

                Bluetooth::Profile::Iterator serviceIdx = _profile.Services();
                while (serviceIdx.Next() == true) {
                    const Bluetooth::Profile::Service& service = serviceIdx.Current();
                    const bool isHidService = (service.Type() == Bluetooth::Profile::Service::HumanInterfaceDevice);
                    const bool isAudioService = (service.Type() == AUDIO_SERVICE_UUID);
                    TRACE(Flow, (_T("[0x%04X] Service: [0x%04X]         %s"), service.Handle(), service.Max(), service.Name().c_str()));

                    Bluetooth::Profile::Service::Iterator characteristicIdx = service.Characteristics();
                    while (characteristicIdx.Next() == true) {
                        const Bluetooth::Profile::Service::Characteristic& characteristic(characteristicIdx.Current());
                        const bool isHidReportCharacteristic = ((isHidService == true) && (characteristic.Type() == Bluetooth::Profile::Service::Characteristic::Report));
                        const bool isAudioCharacteristic = ((isAudioService == true) && ((characteristic.Type() == AUDIO_COMMAND_UUID) || (characteristic.Type() == AUDIO_DATA_UUID)));
                        TRACE(Flow, (_T("[0x%04X]    Characteristic [0x%02X]: %s [%d]"), characteristic.Handle(), characteristic.Rights(), characteristic.Name().c_str(), characteristic.Error()));

                        Bluetooth::Profile::Service::Characteristic::Iterator descriptorIdx = characteristic.Descriptors();
                        while (descriptorIdx.Next() == true) {
                            const Bluetooth::Profile::Service::Characteristic::Descriptor& descriptor(descriptorIdx.Current());
                            const bool isHidReportCharacteristicConfig = ((isHidReportCharacteristic == true) && (descriptor.Type() == Bluetooth::Profile::Service::Characteristic::Descriptor::ClientCharacteristicConfiguration));
                            const bool isAudioCharacteristicConfig = ((isAudioCharacteristic == true) && (descriptor.Type() == Bluetooth::Profile::Service::Characteristic::Descriptor::ClientCharacteristicConfiguration));
                            TRACE(Flow, (_T("[0x%04X]       Descriptor:         %s"), descriptor.Handle(), descriptor.Name().c_str()));

                            if ((isHidReportCharacteristicConfig == true) || (isAudioCharacteristicConfig == true)) {
                                _handles.push_back(descriptor.Handle());
                            }
                        }
                    }
                }
            }
            void EnableEvents(const GATTSocket::Command& cmd)
            {
                if ( (cmd.Error() != Core::ERROR_NONE) || (cmd.Result().Error() != 0) ) {
                    TRACE(Flow, (_T("Enabled reporting on [%04X], Failed: [%d]!!!"), _handles.front(), cmd.Result().Error()));
                }
                else {
                    TRACE(Flow, (_T("Enabled reporting on [%04X], Succeeded"), _handles.front()));
                }

                _handles.pop_front();

                if (_handles.size() > 0) {
                    uint16_t val = htobs(1);
                    _command.Write(_handles.front(), sizeof(val), reinterpret_cast<const uint8_t*>(&val));
                    Execute(CommunicationTimeOut, _command, [&](const GATTSocket::Command& cmd) {
                        EnableEvents(cmd);
                    });
                }
            }
            void Updated()
            {
                _adminLock.Lock();
                if (_device != nullptr) {
                    if (_device->IsConnected() == true) {
                        if ((IsOpen() == false) && (IsConnecting() == false)) {
                            Connect();
                        } else {
                            TRACE(Trace::Information, (_T("Connected GATT socket %s"), _device->RemoteId().c_str()));
                        }
                    } else if (_device->IsValid() == true) {
                        if (IsOpen() == true) {
                            Disconnect();
                        } else {
                            TRACE(Trace::Information, (_T("Disconnected GATT socket %s"), _device->RemoteId().c_str()));
                            _parent->RemoteControlDisconnected(*this);
                        }
                    } else {
                        TRACE(Flow, (_T("Releasing device"), _handles.front()));
                        _device->Release();
                        _device = nullptr;
                    }
                }
                _adminLock.Unlock();
            }

        private:
            Core::CriticalSection _adminLock;
//            state _state;
            Bluetooth::Profile _profile;
            Command _command;
            IBluetooth::IDevice* _device;
            std::list<uint16_t> _handles;
            Core::Sink<Sink> _sink;
            uint16_t _currentKey;
            const Core::ProxyType<Activity> _activity;
            BluetoothControl* _parent;
            string _name;
        };

        class Config : public Core::JSON::Container {
        private:
            Config(const Config&);
            Config& operator=(const Config&);

        public:
            Config()
                : Core::JSON::Container()
                , Interface(0)
                , Name(_T("Thunder BT Control"))
                , External(false)
            {
                Add(_T("interface"), &Interface);
                Add(_T("name"), &Name);
                Add(_T("external"), &External);
            }
            ~Config()
            {
            }

        public:
            Core::JSON::DecUInt16 Interface;
            Core::JSON::String Name;
            Core::JSON::Boolean External;
        };

    public:
        class EXTERNAL DeviceImpl : public IBluetooth::IDevice {
        private:
            class UpdateJob : public Core::IDispatch {
            private:
                UpdateJob() = delete;
                UpdateJob(const UpdateJob&) = delete;
                UpdateJob& operator=(const UpdateJob&) = delete;

            public:
                UpdateJob(DeviceImpl* parent)
                    : _parent(*parent)
                    , _scheduled(0)
                {
                }
                virtual ~UpdateJob()
                {
                }

            public:
                void Schedule()
                {
                    if (_scheduled == 0) {
                        _scheduled = 1;
                        PluginHost::WorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(*this));
                    }
                }
                void Revoke()
                {
                    _scheduled = 0;
                    PluginHost::WorkerPool::Instance().Revoke(Core::ProxyType<Core::IDispatch>(*this));
                }

            public:
                void Dispatch() override
                {
                    _scheduled = 0;
                    _parent.Updated();
                }

            private:
                DeviceImpl& _parent;
                uint8_t _scheduled;
            };

            DeviceImpl() = delete;
            DeviceImpl(const DeviceImpl&) = delete;
            DeviceImpl& operator=(const DeviceImpl&) = delete;

            static constexpr uint16_t ACTION_MASK = 0x0FFF;

        public:
            static constexpr uint32_t MAX_ACTION_TIMEOUT = 2000; /* 2S to setup a connection ? */

            enum state : uint16_t {
                CONNECTING    = 0x0001,
                DISCONNECTING = 0x0002,
                PAIRING       = 0x0004,
                UNPAIRING     = 0x0008,

                PAIRED        = 0x1000,
                BONDED        = 0x2000,
                LOWENERGY     = 0x4000,
                PUBLIC        = 0x8000
            };

            class JSON : public Core::JSON::Container {
            private:
                JSON& operator=(const JSON&);

            public:
                JSON()
                    : Core::JSON::Container()
                    , LocalId()
                    , RemoteId()
                    , Name()
                    , LowEnergy(false)
                    , Connected(false)
                    , Paired(false)
                    , Bonded(false)
                    , Reason(0)
                {
                    Add(_T("local"), &LocalId);
                    Add(_T("remote"), &RemoteId);
                    Add(_T("name"), &Name);
                    Add(_T("le"), &LowEnergy);
                    Add(_T("connected"), &Connected);
                    Add(_T("paired"), &Paired);
                    Add(_T("bonded"), &Bonded);
                    Add(_T("reason"), &Reason);
                }
                JSON(const JSON& copy)
                    : Core::JSON::Container()
                    , LocalId()
                    , RemoteId()
                    , Name()
                    , LowEnergy(false)
                    , Connected(false)
                    , Paired(false)
                    , Bonded(false)
                    , Reason(0)
                {
                    Add(_T("local"), &LocalId);
                    Add(_T("remote"), &RemoteId);
                    Add(_T("name"), &Name);
                    Add(_T("le"), &LowEnergy);
                    Add(_T("connected"), &Connected);
                    Add(_T("paired"), &Paired);
                    Add(_T("bonded"), &Bonded);
                    Add(_T("reason"), &Reason);
                    LocalId = copy.LocalId;
                    RemoteId = copy.RemoteId;
                    Name = copy.Name;
                    LowEnergy = copy.LowEnergy;
                    Connected = copy.Connected;
                    Paired = copy.Paired;
                    Bonded = copy.Bonded;
                    Reason = copy.Reason;
                }
                virtual ~JSON()
                {
                }

            public:
                JSON& Set(const DeviceImpl* source)
                {
                    if (source != nullptr) {
                        LocalId = source->LocalId();
                        RemoteId = source->RemoteId();
                        Name = source->Name();
                        LowEnergy = source->LowEnergy();
                        Connected = source->IsConnected();
                        Paired = source->IsPaired();
                        Bonded = source->IsBonded();
                    } else {
                        LocalId.Clear();
                        RemoteId.Clear();
                        Name.Clear();
                        LowEnergy.Clear();
                        Paired.Clear();
                        Bonded.Clear();
                        Connected.Clear();
                    }
                    return (*this);
                }
                Core::JSON::String LocalId;
                Core::JSON::String RemoteId;
                Core::JSON::String Name;
                Core::JSON::Boolean LowEnergy;
                Core::JSON::Boolean Connected;
                Core::JSON::Boolean Paired;
                Core::JSON::Boolean Bonded;
                Core::JSON::DecUInt16 Reason;
            };

            class IteratorImpl : public IBluetooth::IDevice::IIterator {
            private:
                IteratorImpl() = delete;
                IteratorImpl(const IteratorImpl&) = delete;
                IteratorImpl& operator=(const IteratorImpl&) = delete;

            public:
                IteratorImpl(const std::list<DeviceImpl*>& container)
                {
                    std::list<DeviceImpl*>::const_iterator index = container.begin();
                    while (index != container.end()) {
                        IBluetooth::IDevice* element = (*index);
                        element->AddRef();
                        _list.push_back(element);
                        index++;
                    }
                }
                virtual ~IteratorImpl()
                {
                    while (_list.size() != 0) {
                        _list.front()->Release();
                        _list.pop_front();
                    }
                }

            public:
                virtual void Reset() override
                {
                    _index = 0;
                }
                virtual bool IsValid() const override
                {
                    return ((_index != 0) && (_index <= _list.size()));
                }
                virtual bool Next() override
                {
                    if (_index == 0) {
                        _index = 1;
                        _iterator = _list.begin();
                    } else if (_index <= _list.size()) {
                        _index++;
                        _iterator++;
                    }
                    return (IsValid());
                }
                virtual IBluetooth::IDevice* Current()
                {
                    ASSERT(IsValid() == true);
                    IBluetooth::IDevice* result = nullptr;
                    result = (*_iterator);
                    ASSERT(result != nullptr);
                    result->AddRef();
                    return (result);
                }

                BEGIN_INTERFACE_MAP(IteratorImpl)
                INTERFACE_ENTRY(IBluetooth::IDevice::IIterator)
                END_INTERFACE_MAP

            private:
                uint32_t _index;
                std::list<IBluetooth::IDevice*> _list;
                std::list<IBluetooth::IDevice*>::iterator _iterator;
            };

        public:
            DeviceImpl(const Bluetooth::Address::type type, const uint16_t deviceId, const Bluetooth::Address& remote, const string& name)
                : _name(name)
                , _deviceId(deviceId)
                , _handle(~0)
                , _remote(remote)
                , _state(static_cast<state>((type == Bluetooth::Address::BREDR_ADDRESS      ? 0 :
                                            (type == Bluetooth::Address::LE_RANDOM_ADDRESS  ? LOWENERGY :
                                                                                              LOWENERGY|PUBLIC))))
                , _capabilities(~0)
                , _authentication(~0)
                , _oob_data(~0)
                , _interval(0)
                , _latency(0)
                , _timeout(0)
                , _callback(nullptr)
                , _updateJob(Core::ProxyType<UpdateJob>::Create(this))
            {
                ::memset(_features, 0xFF, sizeof(_features));

            }
            ~DeviceImpl()
            {
                _updateJob->Revoke();
            }

        public:
            uint32_t Callback(IBluetooth::IDevice::ICallback* callback) override
            {
                uint32_t result = Core::ERROR_UNAVAILABLE;
                _state.Lock();
                if (callback == nullptr) {
                    if (_callback != nullptr) {
                        _callback->Release();
                        _callback = nullptr;
                        result = Core::ERROR_NONE;
                    }
                }
                else if (_callback == nullptr) {
                    _callback = callback;
                    _callback->AddRef();
                    result = Core::ERROR_NONE;
                }
                _state.Unlock();
                return (result);
            }
            uint32_t Pair(const IBluetooth::IDevice::capabilities caps) override
            {
                uint32_t result = Core::ERROR_INPROGRESS;

                if (SetState(PAIRING) == Core::ERROR_NONE) {
                    result = BluetoothControl::Connector().Pair(_remote, static_cast<Bluetooth::Address::type>(Type()), static_cast<Bluetooth::ManagementSocket::capabilities>(caps));
                    if (result == Core::ERROR_ALREADY_CONNECTED) {
                        TRACE(Trace::Information, (_T("Already paired")));
                        Paired(true);
                        Bonded(true);
                    } else if (result != Core::ERROR_NONE) {
                        TRACE(Trace::Error, (_T("Failed to pair [%d]"), result));
                        ClearState(PAIRING);
                    }
                }
                return (result);
            }
            uint32_t Unpair() override
            {
                uint32_t result = Core::ERROR_INPROGRESS;

                if (SetState(UNPAIRING) == Core::ERROR_NONE) {
                    result = BluetoothControl::Connector().Unpair(_remote, static_cast<Bluetooth::Address::type>(Type()));
                    if (result == Core::ERROR_NONE) {
                        Paired(false);
                        Bonded(false);
                    } else if (result == Core::ERROR_ALREADY_RELEASED) {
                        TRACE(Trace::Information, (_T("Not paired")));
                    }
                    else {
                        TRACE(Trace::Error, (_T("Failed to unpair [%d]"), result));
                    }
                }
                return (result);
            }
            void Paired(bool paired)
            {
                TRACE(DeviceFlow, (_T("The device [%s] is %spaired"), RemoteId().c_str(), paired? "" : "un"));
                ClearState(PAIRING);
                ClearState(UNPAIRING);
                if (paired == true) {
                    SetState(PAIRED);
                } else {
                    ClearState(PAIRED);
                }
            }
            void Bonded(bool bonded)
            {
                TRACE(DeviceFlow, (_T("The device [%s] is %sbonded"), RemoteId().c_str(), bonded? "" : "un"));
                if (bonded == true) {
                    SetState(BONDED);
                } else {
                    ClearState(BONDED);
                }
            }
            bool IsValid() const override
            {
                return (_state != 0);
            }
            IBluetooth::IDevice::type Type () const override
            {
                return ( (_state & (LOWENERGY|PUBLIC)) == 0         ? IBluetooth::IDevice::ADDRESS_BREDR     :
                         (_state & (LOWENERGY|PUBLIC)) == LOWENERGY ? IBluetooth::IDevice::ADDRESS_LE_RANDOM :
                                                                      IBluetooth::IDevice::ADDRESS_LE_PUBLIC );
            }
            string LocalId() const override
            {
                return (Bluetooth::Address(_deviceId).ToString());
            }
            string RemoteId() const override
            {
                return (_remote.ToString());
            }
            string Name() const override
            {
                return (_name);
            }
            bool IsConnected() const override
            {
                return ( (_handle != static_cast<uint16_t>(~0)) && (_features[0] != 0xFF) && (_features[1] != 0xFF) && (_features[2] != 0xFF) && (_features[3] != 0xFF) );
            }
            bool IsPaired() const override
            {
                return ((_state & PAIRED) != 0);
            }
            bool IsBonded() const override
            {
                return ((_state & BONDED) != 0);
            }
            inline uint16_t DeviceId() const
            {
                return (_deviceId);
            }
            inline bool LowEnergy() const
            {
                return ((_state & LOWENERGY) != 0);
            }
            inline void Clear()
            {
                _state.Lock();
                if ((IsConnected() == false) && (IsPaired() == false) && ((_state & ACTION_MASK) == 0)) {
                    _state.SetState(static_cast<state>(0));
                }
                _state.Unlock();
            }
            inline bool operator==(const Bluetooth::Address& rhs) const
            {
                return (_remote == rhs);
            }
            inline bool operator!=(const Bluetooth::Address& rhs) const
            {
                return (!operator==(rhs));
            }

            BEGIN_INTERFACE_MAP(DeviceImpl)
            INTERFACE_ENTRY(IBluetooth::IDevice)
            END_INTERFACE_MAP

            uint32_t WaitState(const uint32_t state, const uint32_t waitTime)
            {
                return (_state.WaitState(state, waitTime));
            }
            Bluetooth::HCISocket::FeatureIterator Features() const
            {
                return (Bluetooth::HCISocket::FeatureIterator(static_cast<uint8_t>(sizeof(_features)), _features));
            }
            const Bluetooth::Address& Locator() const
            {
                return(_remote);
            }

            void Capabilities(const uint8_t capability, const uint8_t authentication, const uint8_t oob_data)
            {
                _capabilities = capability;
                _authentication = authentication;
                _oob_data = oob_data;
            }
            uint16_t ConnectionId() const
            {
                return (_handle);
            }
        protected:
            friend class ControlSocket;

            uint32_t SetState(const state value)
            {
                uint32_t result = Core::ERROR_INPROGRESS;

                _state.Lock();

                if ((_state & ACTION_MASK) == 0) {

                    result = Core::ERROR_NONE;

                    _state.SetState(static_cast<state>(_state.GetState() | value));
                }

                _state.Unlock();

                return (result);
            }
            uint32_t ClearState(const state value)
            {
                uint32_t result = Core::ERROR_INPROGRESS;

                _state.Lock();

                if ((_state & ACTION_MASK) == value) {

                    result = Core::ERROR_NONE;

                    _state.SetState(static_cast<state>(_state.GetState() & (~value)));
                }

                _state.Unlock();

                return (result);
            }
            void Connection(const uint16_t handle, const uint8_t role)
            {
                _state.Lock();

                if ( (_handle == static_cast<uint16_t>(~0)) ^ (handle == static_cast<uint16_t>(~0)) ) {

                    TRACE(DeviceFlow, (_T("The connection state changed to: %d"), handle));
                    // This is a proper, Connect or disconnect. A real state change..
                    _handle = handle;
                    ClearState(CONNECTING);
                }
                else {
                    TRACE(DeviceFlow, (_T("The connection handle is changed during the runtime. from: %d to: %d"), _handle, handle));
                    _handle = handle;
                }

                _state.Unlock();

                //_updateJob->Schedule();
            }
            void Disconnection(const uint8_t reason)
            {
                _state.Lock();
                TRACE(DeviceFlow, (_T("Disconnected connection %d, reason: %d"), _handle, reason));
                ClearState(CONNECTING);
                ClearState(DISCONNECTING);
                _handle = ~0;
                _state.Unlock();
                _updateJob->Schedule();
            }
            void Name(const string& name)
            {
                TRACE(DeviceFlow, (_T("The device name is updated to: %s"), name.c_str()));
                _name = name;
            }
            void Features(const uint8_t length, const uint8_t feature[])
            {
                uint8_t copyLength = std::min(length, static_cast<uint8_t>(sizeof(_features)));

                _state.Lock();

                if (::memcmp(_features, feature, copyLength) != 0) {
                    if ( (_features[0] == 0xFF) && (_features[1] == 0xFF) && (_features[2] == 0xFF) && (_features[3] == 0xFF) ) {
                        TRACE(DeviceFlow, (_T("The device features are set")));
                    }
                    else {
                        TRACE(DeviceFlow, (_T("The device features are updated")));
                    }

                    ::memcpy(_features, feature, copyLength);
                }
                else {
                    TRACE(DeviceFlow, (_T("Same device features are offered again")));
                }

                _state.Unlock();

                _updateJob->Schedule();
            }

        private:
            void Updated()
            {
                IBluetooth::IDevice::ICallback* callback = nullptr;

                _state.Lock();

                if (_callback != nullptr) {
                    callback = _callback;
                    callback->AddRef();
                }

                _state.Unlock();

                if (callback != nullptr) {
                    callback->Updated();
                    callback->Release();
                }
            }

        private:
            string _name;
            Bluetooth::Address::type _type;
            uint16_t _deviceId;
            uint16_t _handle;
            Bluetooth::Address _remote;
            Core::StateTrigger<state> _state;
            uint8_t _features[8];
            uint8_t _capabilities;
            uint8_t _authentication;
            uint8_t _oob_data;
            uint16_t _interval;
            uint16_t _latency;
            uint16_t _timeout;
            IBluetooth::IDevice::ICallback* _callback;
            Core::ProxyType<UpdateJob> _updateJob;
        };

        class EXTERNAL DeviceRegular : public DeviceImpl {
        private:
            DeviceRegular() = delete;
            DeviceRegular(const DeviceRegular&) = delete;
            DeviceRegular& operator=(const DeviceRegular&) = delete;

        public:
            DeviceRegular(const uint16_t deviceId, const Bluetooth::Address& address, const string& name)
                : DeviceImpl(Bluetooth::Address::BREDR_ADDRESS, deviceId, address, name)
            {
                Bluetooth::HCISocket::Command::RemoteName cmd;

                cmd.Clear();
                cmd->bdaddr = *(Locator().Data());
                cmd->pscan_mode = 0x00;
                cmd->pscan_rep_mode = 0x02;
                cmd->clock_offset = 0x0000;
                BluetoothControl::Connector().Execute<Bluetooth::HCISocket::Command::RemoteName>(MAX_ACTION_TIMEOUT, cmd, [&](Bluetooth::HCISocket::Command::RemoteName& cmd, const uint32_t error) {
                    if (error == Core::ERROR_NONE) {
                        Update(cmd);
                    }
                });
            }
            virtual ~DeviceRegular()
            {
            }

        public:
            virtual uint32_t Connect() override
            {
                uint32_t result = Core::ERROR_INPROGRESS;

                if (SetState(CONNECTING) == Core::ERROR_NONE) {
                    Bluetooth::HCISocket::Command::Connect connect;

                    connect.Clear();
                    connect->bdaddr = *(Locator().Data());
                    connect->pkt_type = htobs(HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5);
                    connect->pscan_rep_mode = 0x02;
                    connect->clock_offset = 0x0000;
                    connect->role_switch = 0x01;

                    result = BluetoothControl::Connector().Exchange(MAX_ACTION_TIMEOUT, connect, connect);

                    if (result == Core::ERROR_NONE) {
                        Connection (connect.Response().handle, 0);
                    }
                    else {
                        TRACE(ControlFlow, (_T("Failed to connect. Error [%d]"), result));
                    }

                    ClearState(CONNECTING);
                }

                return (result);
            }
            virtual uint32_t Disconnect(const uint16_t reason) override
            {
                uint32_t result = Core::ERROR_INPROGRESS;

                if (SetState(DISCONNECTING) == Core::ERROR_NONE) {
                    Bluetooth::HCISocket::Command::Disconnect disconnect;

                    disconnect->handle = htobs(ConnectionId());
                    disconnect->reason = (reason & 0xFF);

                    result = BluetoothControl::Connector().Exchange(MAX_ACTION_TIMEOUT, disconnect, disconnect);

                    if (result != Core::ERROR_NONE) {
                        TRACE(ControlFlow, (_T("Failed to disconnect. Error [%d]"), result));
                    }
                }

                return (result);
            }

        private:
            void Update(Bluetooth::HCISocket::Command::RemoteName& data)
            {
                // Metadata is flowing in, handle it..
                // _cmds.name.Response().bdaddr;
                const char* longName = reinterpret_cast<const char*>(data.Response().name);
                uint8_t index = 0;
                printf("UPDATED => ");
                while (index < HCI_MAX_NAME_LENGTH) {
                    printf("%c", ::isprint(longName[index]) ? longName[index] : '.');
                    index++;
                }
                printf("\n");
                index = 0;
                while ((index < HCI_MAX_NAME_LENGTH) && (::isprint(longName[index]) != 0)) {
                    index++;
                }

                Name(std::string(longName, index));
                TRACE(ControlFlow, (_T("Loaded Long Device Name: %s"),longName));
            }
        };

        class EXTERNAL DeviceLowEnergy : public DeviceImpl, Core::IOutbound::ICallback {
        private:
            DeviceLowEnergy() = delete;
            DeviceLowEnergy(const DeviceLowEnergy&) = delete;
            DeviceLowEnergy& operator=(const DeviceLowEnergy&) = delete;

        public:
            DeviceLowEnergy(const uint16_t deviceId, const Bluetooth::Address& address, const string& name)
                : DeviceImpl(Bluetooth::Address::LE_PUBLIC_ADDRESS, deviceId, address, name)
            {
            }
            virtual ~DeviceLowEnergy()
            {
            }

        public:
            virtual uint32_t Connect() override
            {
                uint32_t result = Core::ERROR_INPROGRESS;

                if (SetState(CONNECTING) == Core::ERROR_NONE) {

                    Bluetooth::HCISocket::Command::ConnectLE connect;
                    connect.Clear();
                    connect->interval = htobs(0x0004);
                    connect->window = htobs(0x0004);
                    connect->initiator_filter = 0;
                    connect->peer_bdaddr_type = LE_PUBLIC_ADDRESS;
                    connect->peer_bdaddr = *(Locator().Data());
                    connect->own_bdaddr_type = LE_PUBLIC_ADDRESS;
                    connect->min_interval = htobs(0x000F);
                    connect->max_interval = htobs(0x000F);
                    connect->latency = htobs(0x0000);
                    connect->supervision_timeout = htobs(0x0C80);
                    connect->min_ce_length = htobs(0x0001);
                    connect->max_ce_length = htobs(0x0001);

                    result = BluetoothControl::Connector().Exchange(MAX_ACTION_TIMEOUT, connect, connect);
                    if (result != Core::ERROR_NONE) {
                        TRACE(ControlFlow, (_T("Failed to connect. Error [%d]"), result));
                        ClearState(CONNECTING);
                    }
                }

                TRACE(ControlFlow, (_T("DeviceLowEnergy Connect() status <%i>"), result));

                return (result);
            }
            virtual uint32_t Disconnect(const uint16_t reason) override
            {
                uint32_t result = Core::ERROR_INPROGRESS;

                if (SetState(DISCONNECTING) == Core::ERROR_NONE) {

                    Bluetooth::HCISocket::Command::Disconnect disconnect;
                    disconnect->handle = htobs(ConnectionId());
                    disconnect->reason = reason & 0xFF;

                    result = BluetoothControl::Connector().Exchange(MAX_ACTION_TIMEOUT, disconnect, disconnect);
                    if (result != Core::ERROR_NONE) {
                        TRACE(ControlFlow, (_T("Failed to disconnect. Error [%d]"), result));
                        ClearState(DISCONNECTING);
                    }
                }

                return (result);
            }

        private:
            virtual void Updated(const Core::IOutbound& data, const uint32_t error_code) override
            {
                printf (" %d ------------- %s ------\n", __LINE__, __FUNCTION__);
            }
        };

        class EXTERNAL Status : public Core::JSON::Container {
        private:
            Status(const Status&) = delete;
            Status& operator=(const Status&) = delete;

        public:
            class EXTERNAL Property : public Core::JSON::Container {
            public:
                Property()
                    : Name()
                    , Supported(false)
                    , Enabled(false) {
                    Add(_T("name"), &Name);
                    Add(_T("supported"), &Supported);
                    Add(_T("enabled"), &Enabled);
                }
                Property(const string& name, const bool supported, const bool enabled)
                    : Name(name)
                    , Supported(supported)
                    , Enabled(enabled) {
                    Add(_T("name"), &Name);
                    Add(_T("supported"), &Supported);
                    Add(_T("enabled"), &Enabled);
                    Name = name;
                    Supported = supported;
                    if (supported == true) {
                        Enabled = enabled;
                    }
                }
                Property(const Property& copy)
                    : Name(copy.Name)
                    , Supported(copy.Supported)
                    , Enabled(copy.Enabled) {
                    Add(_T("name"), &Name);
                    Add(_T("supported"), &Supported);
                    Add(_T("enabled"), &Enabled);
                }
                virtual ~Property() {
                }

            public:
                Core::JSON::String Name;
                Core::JSON::Boolean Supported;
                Core::JSON::Boolean Enabled;
            };

        public:
            Status()
                : Scanning()
                , Devices()
                , Name()
                , Version(0)
                , Address()
                , DeviceClass(0)
            {
                Add(_T("scanning"), &Scanning);
                Add(_T("devices"), &Devices);
                Add(_T("properties"), &Properties);
            }
            virtual ~Status()
            {
            }

            void AddProperty(const string& name, const bool supported, const bool enabled) {
                Property newEntry (name, supported, enabled);
                Properties.Add(newEntry);
            }

        public:
            Core::JSON::Boolean Scanning;
            Core::JSON::ArrayType<DeviceImpl::JSON> Devices;
            Core::JSON::String Name;
            Core::JSON::DecUInt32 Version;
            Core::JSON::String Address;
            Core::JSON::DecUInt32 DeviceClass;
            Core::JSON::ArrayType<Property> Properties;
        };

    public:
        BluetoothControl(const BluetoothControl&) = delete;
        BluetoothControl& operator=(const BluetoothControl&) = delete;
        BluetoothControl()
            : _skipURL(0)
            , _adminLock()
            , _service(nullptr)
            , _btInterface(0)
            , _btAddress()
            , _devices()
            , _observers()
            , _gattRemote(nullptr)
            , _linkKeys()
            , _longTermKeys()
            , _identityKeys()
            , _signatureKeys()
            , _linkKeysFile()
            , _longTermKeysFile()
            , _identityKeysFile()
            , _signatureKeysFile()
            , _inputHandler(PluginHost::InputHandler::Handler())
        {
            ASSERT(_inputHandler != nullptr);
        }
        virtual ~BluetoothControl()
        {
        }

    public:
        BEGIN_INTERFACE_MAP(BluetoothControl)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(Exchange::IBluetooth)
        INTERFACE_ENTRY(Exchange::IKeyHandler)
        END_INTERFACE_MAP

    public:
        //  IPlugin methods
        // -------------------------------------------------------------------------------------------------------

        // First time initialization. Whenever a plugin is loaded, it is offered a Service object with relevant
        // information and services for this particular plugin. The Service object contains configuration information that
        // can be used to initialize the plugin correctly. If Initialization succeeds, return nothing (empty string)
        // If there is an error, return a string describing the issue why the initialisation failed.
        // The Service object is *NOT* reference counted, lifetime ends if the plugin is deactivated.
        // The lifetime of the Service object is guaranteed till the deinitialize method is called.
        virtual const string Initialize(PluginHost::IShell* service);

        // The plugin is unloaded from the webbridge. This is call allows the module to notify clients
        // or to persist information if needed. After this call the plugin will unlink from the service path
        // and be deactivated. The Service object is the same as passed in during the Initialize.
        // After this call, the lifetime of the Service object ends.
        virtual void Deinitialize(PluginHost::IShell* service);

        // Returns an interface to a JSON struct that can be used to return specific metadata information with respect
        // to this plugin. This Metadata can be used by the MetaData plugin to publish this information to the outside world.
        virtual string Information() const;

        //  IWeb methods
        // -------------------------------------------------------------------------------------------------------
        virtual void Inbound(Web::Request& request);
        virtual Core::ProxyType<Web::Response> Process(const Web::Request& request);

        //  IBluetooth methods
        // -------------------------------------------------------------------------------------------------------
        virtual bool IsScanning() const override;
        virtual uint32_t Register(IBluetooth::INotification* notification) override;
        virtual uint32_t Unregister(IBluetooth::INotification* notification) override;

        virtual bool Scan(const bool enable) override;
        virtual IBluetooth::IDevice* Device(const string&) override;
        virtual IBluetooth::IDevice::IIterator* Devices() override;

        // IKeyHandler methods
        uint32_t KeyEvent(const bool pressed, const uint32_t code, const string& mapName) override
        {
            return (_inputHandler->KeyEvent(pressed, code, mapName));
        }
        virtual Exchange::IKeyProducer* Producer(const string& name) override
        {
            return (_gattRemote);
        }
        void RemoteControlConnected(GATTRemote& remote)
        {
            TRACE(Trace::Information, (_T("Bluetooth LE remote control unit \"%s\" connected"), remote.Name()));
            string path(_service->DataPath() + string(remote.Name()) + "-remote.json");
            PluginHost::VirtualInput::KeyMap& map(_inputHandler->Table(remote.Name()));
            TRACE(Trace::Information, (_T("Loading keymap file %s"), path.c_str()));
            if (map.Load(path) != Core::ERROR_NONE) {
                TRACE(Trace::Error, (_T("Failed to load keymap file %s; attempting pass-through"),path.c_str()));
                map.PassThrough(true);
            }
        }
        void RemoteControlDisconnected(GATTRemote& remote)
        {
            TRACE(Trace::Information, (_T("Bluetooth LE remote control unit \"%s\" disconnected"), remote.Name()));
            _inputHandler->ClearTable(remote.Name());
        }

    public:
        inline static ControlSocket& Connector() {
            return (_application);
        }

        void StoreKey(const Bluetooth::LinkKey& key) {
            _adminLock.Lock();
            _linkKeys.Add(key);
            _adminLock.Unlock();
        }
        void StoreKey(const Bluetooth::LongTermKey& key) {
            _adminLock.Lock();
            _longTermKeys.Add(key);
            _adminLock.Unlock();
        }
        void StoreKey(const Bluetooth::IdentityKey& key) {
            _adminLock.Lock();
            _identityKeys.Add(key);
            _adminLock.Unlock();
        }
        void StoreKey(const Bluetooth::SignatureKey& key) {
            _adminLock.Lock();
            _signatureKeys.Add(key);
            _adminLock.Unlock();
        }

        void PurgeDeviceKeys(const Bluetooth::Address& device);

        uint32_t SaveLeEncryptionKeys();
        uint32_t SaveEdrEncryptionKeys();

    private:
        Core::ProxyType<Web::Response> GetMethod(Core::TextSegmentIterator& index);
        Core::ProxyType<Web::Response> PutMethod(Core::TextSegmentIterator& index, const Web::Request& request);
        Core::ProxyType<Web::Response> PostMethod(Core::TextSegmentIterator& index, const Web::Request& request);
        Core::ProxyType<Web::Response> DeleteMethod(Core::TextSegmentIterator& index, const Web::Request& request);
        void RemoveDevices(std::function<bool(DeviceImpl*)> filter);
        void Discovered(const bool lowEnergy, const Bluetooth::Address& address, const string& name);
        void Notification(const uint8_t subEvent, const uint16_t length, const uint8_t* dataFrame);
        DeviceImpl* Find(const Bluetooth::Address&);
        DeviceImpl* Find(const uint16_t handle);
        void Capabilities(const Bluetooth::Address& device, const uint8_t capability, const uint8_t authentication, const uint8_t oob_data);
        void LoadEncryptionKeys();

    private:
        uint8_t _skipURL;
        Core::CriticalSection _adminLock;
        PluginHost::IShell* _service;
        uint16_t _btInterface;
        Bluetooth::Address _btAddress;
        std::list<DeviceImpl*> _devices;
        std::list<IBluetooth::INotification*> _observers;
        GATTRemote* _gattRemote;
        Config _config;
        static ControlSocket _application;
        string _persistentStoragePath;
        Bluetooth::LinkKeys _linkKeys;
        Bluetooth::LongTermKeys _longTermKeys;
        Bluetooth::IdentityKeys _identityKeys;
        Bluetooth::SignatureKeys _signatureKeys;
        Core::File _linkKeysFile;
        Core::File _longTermKeysFile;
        Core::File _identityKeysFile;
        Core::File _signatureKeysFile;
        PluginHost::VirtualInput* _inputHandler;
    };

} //namespace Plugin

}
