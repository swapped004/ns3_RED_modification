/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include <ns3/lr-wpan-module.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/packet.h>
#include <iostream>
#include "ns3/traffic-control-module.h"
#include "ns3/trace-helper.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv6-header.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;
//using namespace std;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 10;
  uint32_t nWifi = 10;
  bool tracing = false;
  std::string p2p_datarate = "1Mbps";
  std::string p2p_delay = "5ms";
  uint32_t nFlows = 20;
  uint32_t port = 4001;
  uint32_t pktSize = 512;
  uint32_t    maxPackets = 100;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("p2p_dataRate", "Data rate of p2p link", p2p_datarate);
  cmd.AddValue ("nFlows", "Number of Flows", nFlows);
  cmd.AddValue ("pktSize", "Size of each packet sent", pktSize);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

     Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (pktSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("10Mbps"));

  Config::SetDefault ("ns3::DropTailQueue<Packet>::MaxSize",
                      StringValue (std::to_string (maxPackets) + "p"));

    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(p2p_datarate));
    pointToPoint.SetChannelAttribute("Delay", StringValue(p2p_delay));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);



    NodeContainer csmaNodes;
    csmaNodes.Add (p2pNodes.Get (1));
    csmaNodes.Create (nCsma);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (csmaNodes);

    NodeContainer wifiStaNodes;
    wifiStaNodes.Add(p2pNodes.Get(0));
    wifiStaNodes.Create (nWifi);

    //set channel
    Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
    Ptr<LogDistancePropagationLossModel> propModel = CreateObject<LogDistancePropagationLossModel> ();
    Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
    channel->AddPropagationLossModel (propModel);
    channel->SetPropagationDelayModel (delayModel);

    // LrWpanAddressMode


    LrWpanHelper lrWpanHelper;
    NetDeviceContainer wifiNetDeviceContainer;

    // for(int i=0;i<nWifi;i++)
    // {
    //    Ptr<LrWpanNetDevice> dev = CreateObject<LrWpanNetDevice> ();
    //    dev->SetChannel(channel);
    //    wifiStaNodes.Get(i)->AddDevice(dev);
    //    wifiNetDeviceContainer.Add(dev);
    // }

    lrWpanHelper.SetChannel(channel);
    wifiNetDeviceContainer = lrWpanHelper.Install(wifiStaNodes);

    //associate PAN
    lrWpanHelper.AssociateToPan(wifiNetDeviceContainer, 0);


    //add static property to all the wifi nodes
    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    mobility.Install (wifiStaNodes);


    //set IPV6 to all the nodes and disable IPV4
    InternetStackHelper stack;
    stack.SetIpv4StackInstall(false);

    stack.Install(csmaNodes);
    stack.Install(wifiStaNodes);

    //6lowpan
    SixLowPanHelper sixLowPanHelper;
    NetDeviceContainer sixLowPanDevices = sixLowPanHelper.Install(wifiNetDeviceContainer);

    //add Queue disc methods in TCP layer
    TrafficControlHelper tchBottleneck;
    QueueDiscContainer queueDiscs_right;
    QueueDiscContainer queueDiscs_left;
    tchBottleneck.SetRootQueueDisc ("ns3::RedQueueDisc");
    queueDiscs_left = tchBottleneck.Install (csmaNodes.Get(0)->GetDevice(0));
    queueDiscs_right = tchBottleneck.Install (wifiStaNodes.Get(0)->GetDevice(0));


    //assign IPV6 addresses
    Ipv6AddressHelper address;
    address.SetBase (Ipv6Address ("2001:f00d::"), Ipv6Prefix (64));
    Ipv6InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);
    p2pInterfaces.SetForwarding(1,true);
    p2pInterfaces.SetDefaultRouteInAllNodes(1);
    p2pInterfaces.SetForwarding(0,true);
    p2pInterfaces.SetDefaultRouteInAllNodes(0);


    Ipv6InterfaceContainer csmaInterfaces;
    address.SetBase (Ipv6Address ("2001:cafe::"), Ipv6Prefix (64));
    csmaInterfaces = address.Assign(csmaDevices);
    csmaInterfaces.SetForwarding(0,true);
    csmaInterfaces.SetDefaultRouteInAllNodes(0);

    Ipv6InterfaceContainer wifiInterfaces;
    address.SetBase (Ipv6Address ("2001:b00c::"), Ipv6Prefix (64));
    wifiInterfaces = address.Assign(sixLowPanDevices);
    wifiInterfaces.SetForwarding(0,true);
    wifiInterfaces.SetDefaultRouteInAllNodes(0);

    //mesh
    for(uint32_t i=0;i<sixLowPanDevices.GetN();i++)
    {
      Ptr<NetDevice> dev = sixLowPanDevices.Get(i);
      dev->SetAttribute("UseMeshUnder", BooleanValue(true));
      dev->SetAttribute("MeshUnderRadius", UintegerValue(10));
    }

    //set the applications and flow

    //onoffhelper
    // OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address());
    // clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000]"));
    // clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    
    // Address sinkLocalAddress()

    //ping6
    // Ping6Helper client;
    // /* remote address is first routers in RH0 => source routing */
    // client.SetRemote (wifiInterfaces.GetAddress (0, 0));
    // client.SetAttribute ("MaxPackets", UintegerValue (100));
    // client.SetAttribute ("Interval", TimeValue (Seconds(1)));
    // client.SetAttribute ("PacketSize", UintegerValue (512));
    // // client.SetRoutersAddress (routersAddress);
    // ApplicationContainer apps = client.Install(csmaNodes);
    // apps.Start (Seconds (1.0));
    // apps.Stop (Seconds (10.0));


    //UDP
    //UdpEchoServerHelper echoServer (9);

    // ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (1));
    // serverApps.Start (Seconds (1.0));
    // serverApps.Stop (Seconds (100.0));

    // UdpEchoClientHelper echoClient (wifiInterfaces.GetAddress(1,0),9);
    // echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
    // echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.2)));
    // echoClient.SetAttribute ("PacketSize", UintegerValue (512));

    // ApplicationContainer clientApps = 
    // echoClient.Install (csmaNodes.Get (nCsma));
    // clientApps.Start (Seconds (2.0));
    // clientApps.Stop (Seconds (90.0));


    // ping6
    // uint32_t packetSize = 10;
    // uint32_t maxPacketCount = 11211;
    // Time interPacketInterval = Seconds (0.05);
    // Ping6Helper ping6;
    
    // ping6.SetLocal (csmaInterfaces.GetAddress(nCsma-1,1));
    // ping6.SetRemote (wifiInterfaces.GetAddress (nWifi-1, 1));
    
    // ping6.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
    // ping6.SetAttribute ("Interval", TimeValue (interPacketInterval));
    // ping6.SetAttribute ("PacketSize", UintegerValue (packetSize));
    // ApplicationContainer apps = ping6.Install (csmaNodes.Get (nCsma-1));
    
    // apps.Start (Seconds (1.0));
    // apps.Stop (Seconds (100.0));

    //Install on/off app on all right side nodes
    OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
    clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000]"));
    clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    
    Address sinkLocalAddress (Inet6SocketAddress(Ipv6Address::GetAny(), port));
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApps;
    
    for (uint32_t i = 1; i <= nWifi; ++i)
      {
        sinkApps.Add (packetSinkHelper.Install (wifiStaNodes.Get(i)));
      }
    sinkApps.Start (Seconds (1.0));
    sinkApps.Stop (Seconds (30.0));

    ApplicationContainer clientApps;
    uint32_t cnt=0;
    for (uint32_t i = 1; i <= nCsma; ++i)
      {
        // Create an on/off app sending packets to the left side
        AddressValue remoteAddress (Inet6SocketAddress(wifiInterfaces.GetAddress(i,1), port));
        clientHelper.SetAttribute ("Remote", remoteAddress);
        clientApps.Add (clientHelper.Install (csmaNodes.Get(i)));

        cnt++;

        if(cnt >= nFlows/2 && nFlows != 0)
        {
          break;
        }
      }

    clientApps.Start (Seconds (2.0)); // Start 1 second after sink
    clientApps.Stop (Seconds (20.0)); // Stop before the sink






    //tracing
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor;
    monitor=flowmon.InstallAll();
    


    Simulator::Stop (Seconds (100.0));

      if (tracing)
        {
          csma.EnablePcap ("csma", csmaDevices.Get (0), true);
          lrWpanHelper.EnablePcap("lrwpan", wifiNetDeviceContainer.Get(nWifi-1), true);
        }

    Simulator::Run ();

    QueueDisc::Stats st = queueDiscs_right.Get (0)->GetStats ();
    QueueDisc::Stats st1 = queueDiscs_left.Get (0)->GetStats ();

    if (st.GetNDroppedPackets (RedQueueDisc::UNFORCED_DROP) == 0)
      {
        std::cout << "There should be some unforced drops" << std::endl;
        //exit (1);
      }

    if (st.GetNDroppedPackets (QueueDisc::INTERNAL_QUEUE_DROP) != 0)
      {
        std::cout << "There should be zero drops due to queue full" << std::endl;
        //exit (1);
      }

    std::cout << "*** Stats from the bottleneck queue(right) disc ***" << std::endl;
    std::cout << st << std::endl;
    std::cout << "*** Stats from the bottleneck queue(left) disc ***" << std::endl;
    std::cout << st1 << std::endl;
    std::cout << "Destroying the simulation" << std::endl;


    monitor->SerializeToXmlFile("red_wireless.xml", false, false);




    Simulator::Destroy ();
    return 0;
}
