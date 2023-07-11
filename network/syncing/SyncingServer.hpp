#pragma once  // Header guard see https://en.wikipedia.org/wiki/Include_guard

// Copyright (c) 2023 Sri Lakshmi Kanthan P
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QSslConfiguration>
#include <QSslServer>
#include <QSslSocket>
#include <QVector>

#include "network/discovery/DiscoveryServer.hpp"
#include "types/enums/enums.hpp"
#include "utility/functions/ipconv.hpp"

namespace srilakshmikanthanp::clipbirdesk::network::syncing {
/**
 * @brief Syncing server that syncs the clipboard data between
 * the clients
 */
class SyncingServer : public discovery::DiscoveryServer {
 signals:    // signals
  /// @brief On client state changed
  void OnCLientStateChanged(QSslSocket *client, bool connected);

 signals:  // signals for this class
  /// @brief On Error Occurred
  void OnErrorOccurred(QString error);

 signals:    // signals
  /// @brief On Sync Request
  void OnSyncRequest(QVector<QPair<QString, QByteArray>> items);

 private:    // just for Qt
  /// @brief Qt meta object
  Q_OBJECT

 public:     // Authenticator Type
  /// @brief Authenticator
  using Authenticator = bool (*)(QSslSocket *client);

 private:    // members of the class
  /// @brief SSL server
  QSslServer m_ssl_server = QSslServer(this);

  /// @brief List of clients
  QList<QSslSocket *> m_clients;

 private:    // Authenticator Instance
  /// @brief Authenticator
  Authenticator m_authenticator = nullptr;

 private:    // some typedefs
  using MalformedPacket = types::except::MalformedPacket;

 private:
  /**
   * @brief set up the client that has been connected
   *
   * @param client Client
   */
  void setUpAuthenticatedClient(QSslSocket *client) {
    // Connect the socket to the callback function that
    // process the ready read when the socket is ready
    // to read so the listener can be notified
    const auto signal_r = &QSslSocket::readyRead;
    const auto slot_r = &SyncingServer::processReadyRead;
    QObject::connect(client, signal_r, this, slot_r);

    // Connect the client to the callback function that process
    // the disconnection when the client is disconnected
    // so the listener can be notified
    const auto signal_d = &QSslSocket::disconnected;
    const auto slot_d = &SyncingServer::processDisconnection;
    QObject::connect(client, signal_d, this, slot_d);

    // Add the client to the list of clients
    m_clients.append(client);

    // Notify the listeners that the client has been connected
    emit OnCLientStateChanged(client, true);
  }

  /**
   * @brief Send the malformed packet to the client
   *
   * @param e Exception to construct the malformed packet
   */
  QByteArray constructInvalidPacket(const MalformedPacket& e) {
    // using the ResponseType from the MalformedPacket
    const auto pakType = packets::InvalidPacket::PacketType::MalformedPacket;

    // using the functions namespace
    using namespace srilakshmikanthanp::clipbirdesk::utility::functions;

    // Create the malformed packet to send
    packets::InvalidPacket invalidPacket = createPacket({
      pakType, e.getCode(), e.what(),
    });

    // Create the datagram and return it
    return toQByteArray(invalidPacket);
  }

  /**
   * @brief Send the SyncingPacket to all the clients
   *
   * @param packet SyncingPacket
   */
  void requestClientsToSync(const packets::SyncingPacket& packet) {
    // using the functions namespace
    using namespace srilakshmikanthanp::clipbirdesk::utility::functions;

    // Create the datagram and return it
    const auto data = toQByteArray(packet);

    // Send the data to all the clients
    for (auto client : m_clients) { client->write(data); }
  }

  /**
   * @brief Callback function that process pending
   * connections when the server is ready to accept
   */
  void processConnections() {
    while (m_ssl_server.hasPendingConnections()) {
      // Get the client that has been connected
      auto client_tcp = (m_ssl_server.nextPendingConnection());

      // Convert the client to SSL client
      auto client = qobject_cast<QSslSocket *>(client_tcp);

      // Authenticate the client
      if (!m_authenticator(client)) {
        client->disconnectFromHost();
        client->deleteLater();
      }

      // set up the client
      setUpAuthenticatedClient(client);
    }
  }

  /**
   * @brief Callback function that process the ready
   * read from the client
   */
  void processReadyRead() {
    // Get the client that was ready to read
    auto client = qobject_cast<QSslSocket *>(sender());

    // Get the data from the client
    const auto data = client->readAll();

    // using the fromQByteArray from namespace
    using utility::functions::fromQByteArray;

    // Create the SyncingPacket
    packets::SyncingPacket packet;

    // Deserialize the data to SyncingPacket
    try {
      packet = fromQByteArray<packets::SyncingPacket>(data);
    } catch (const types::except::MalformedPacket &e) {
      client->write(constructInvalidPacket(e));
      return;
    } catch (const std::exception &e) {
      emit OnErrorOccurred(e.what());
      return;
    } catch (...) {
      emit OnErrorOccurred("Unknown Error");
      return;
    }

    // send the SyncingPacket to all the clients
    requestClientsToSync(packet);

    // Make the vector of QPair<QString, QByteArray>
    QVector<QPair<QString, QByteArray>> items;

    // Get the items from the packet
    for (auto item : packet.getItems()) {
      items.append({
        item.getMimeType().toStdString().c_str(), item.getPayload()
      });
    }

    // Notify the listeners to sync the data
    emit OnSyncRequest(items);
  }

  /**
   * @brief Callback function that process the client
   * disconnection
   */
  void processDisconnection() {
    // Get the client that has been disconnected
    auto client = qobject_cast<QSslSocket *>(sender());

    // Remove the client from the list of clients
    m_clients.removeOne(client);

    // Delete the client
    client->deleteLater();

    // Notify the listeners client state
    emit OnCLientStateChanged(client, false);
  }

 public:
  /**
   * @brief Construct a new Syncing Server object and
   * bind to any available port and any available
   * IP address
   *
   * @param config SSL configuration
   * @param parent Parent object
   */
  explicit SyncingServer(const QSslConfiguration &config,
                         Authenticator authenticator,
                         QObject *parent = nullptr)
      : DiscoveryServer(parent) {
    // Connect the socket to the callback function that
    // process the connections when the socket is ready
    // to read so the listener can be notified
    const auto signal_c = &QSslServer::pendingConnectionAvailable;
    const auto slot_c = &SyncingServer::processConnections;
    QObject::connect(&m_ssl_server, signal_c, this, slot_c);

    // set the configuration For the SSL server
    m_ssl_server.setSslConfiguration(config);

    // Set the authenticator
    m_authenticator = authenticator;

    // bind the server to any available port and any available
    // IP address and start listening for connections
    m_ssl_server.listen(QHostAddress::Any);
  }

  /**
   * @brief Destroy the Syncing Server object
   */
  virtual ~SyncingServer() = default;

 protected:  // functions
  /**
   * @brief Get the IP Type of the SyncingServer it can be IPv4 or
   * IPv6 the IP type is used to determine the length of the IP address
   * if the IP type is IPv4 then the IP address is 4 bytes long if
   * the IP type is IPv6 then the IP address is 16 bytes long
   *
   * @note The IP Type used in SyncingServer is IPv4
   *
   * @return types::IPType IP type
   * @throw Any Exception If any error occurs
   */
  virtual types::enums::IPType getIPType() const override {
    return types::enums::IPType::IPv4;
  }

  /**
   * @brief Get the Port number of the SyncingServer it can be any port
   * number from 0 to 65535 but the port number should be greater than 1024
   * because the port number less than 1024 are reserved for the system
   * services
   *
   * @return types::Port Port number
   * @throw Any Exception If any error occurs
   */
  virtual quint16 getPort() const override {
    return m_ssl_server.serverPort();
  }

  /**
   * @brief Get the IP Address of the SyncingServer it can be IPv4 or
   * IPv6 if the IP type is IPv4 then the IP address is 4 bytes long if
   * the IP type is IPv6 then the IP address is 16 bytes long
   *
   * @return types::IPAddress IP address
   * @throw Any Exception If any error occurs
   */
  virtual QHostAddress getIPAddress() const override {
    return m_ssl_server.serverAddress();
  }

  /**
   * @brief On Error Occurred this function is called when any error
   * occurs in the socket
   *
   * @param error Error message
   */
  virtual void OnErrorOccurred(QString error) override {
    emit OnErrorOccurred(error);
  }

 public:
  /**
   * @brief Request the clients to sync the clipboard items
   *
   * @param data QVector<QPair<QString, QByteArray>>
   */
  void requestClientsToSync(QVector<QPair<QString, QByteArray>> items) {
    // using create Packet form the utility::functions namespace
    packets::SyncingPacket packet = utility::functions::createPacket({
      packets::SyncingPacket::PacketType::SyncPacket, items,
    });

    // send the SyncingPacket to all the clients
    this->requestClientsToSync(packet);
  }

  /**
   * @brief Get the Clients that are connected to the server
   *
   * @return QList<QSslSocket*> List of clients
   */
  QList<QSslSocket *> getConnectedClientsList() const {
    return m_clients;
  }
};
}  // namespace srilakshmikanthanp::clipbirdesk::network::syncing
