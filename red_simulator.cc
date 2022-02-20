/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 NITK Surathkal
 *
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
 *
 * Author: Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 *
 */

#include<stdlib.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/trace-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

#include <iostream>
#include <iomanip>
#include <map>
#include<fstream>





using namespace ns3;

std::ofstream out1,out2;



FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor;
double timer=1;
uint32_t prev_packets = 0;
uint32_t prev_bytes = 0;
uint32_t prev_drops = 0;
uint32_t prev_sent = 0;
//file

void throughput() {
  
  monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  uint32_t total_packets=0;
  uint32_t total_bytes = 0;
  uint32_t total_sent=0;
  

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      // first 2 FlowIds are for ECHO apps, we don't want to display them
      //
      // Duration for throughput measurement is 9.0 seconds, since
      //   StartTime of the OnOffApplication is at about "second 1"
      // and
      //   Simulator::Stops at "second 10".
      // if (i->first > 2)
      //   {
      //     Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      //     std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
      //     std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
      //     std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      //     std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
      //     std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
      //     std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      //     std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
      //   }

        total_packets+=i->second.rxPackets;
        total_bytes+=i->second.rxBytes;

        total_sent+=i->second.txPackets;

    }

    uint32_t total_drops = total_sent-total_packets;
    //uint32_t new_drops = total_drops-prev_drops;
    prev_drops = total_drops;

    //uint32_t newtotal_packets= total_packets - prev_packets;
   

    uint32_t newtotal_bytes = total_bytes - prev_bytes;
    prev_bytes = total_bytes;

    double drop_ratio =0;
    if(total_packets != 0)
      {
        drop_ratio = total_drops*100.0/(total_packets+total_drops);
      }


    
    // std::cout << "  total_packets: " << total_packets<< " \n";
    // std::cout << "  Throughput: " << (newtotal_bytes/0.5)<< " Bps\n";
    // std::cout << "packets dropped " << new_drops <<"\n";
    // std::cout << "  Packet drop Ratio: " << drop_ratio<< " % \n";

    prev_packets=total_packets;
    prev_sent = total_sent;


    if(newtotal_bytes != 0){
      out1<<"\t"<<timer<<" "<<(newtotal_bytes/0.5)<<std::endl;
      out2<<"\t"<<timer<<" "<<drop_ratio<<std::endl;
    }

    timer+=0.5;

    Simulator::Schedule(Seconds(0.5), &throughput);
}



int main (int argc, char *argv[])
{
  uint32_t    nLeaf = 10;
  uint32_t    maxPackets = 100;
  bool        modeBytes  = false;
  uint32_t    queueDiscLimitPackets = 1000;
  double      minTh = 5;
  double      maxTh = 15;
  uint32_t    pktSize = 512;
  std::string appDataRate = "10Mbps";
  std::string queueDiscType = "RED";
  uint16_t port = 5001;
  std::string bottleNeckLinkBw = "1Mbps";
  std::string bottleNeckLinkDelay = "50ms";
  uint16_t    clientapp_simulation_time = 14; 

  //added parameters
  bool        tracing=false;
  uint16_t    nFlows=0;
  bool        animation=false;

  uint32_t   flow_maxPackets_sent=0;
  
  

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nLeaf",     "Number of left and right side leaf nodes", nLeaf);
  cmd.AddValue ("maxPackets","Max Packets allowed in the device queue", maxPackets); //buffer size(device)
  cmd.AddValue ("queueDiscLimitPackets","Max Packets allowed in the queue disc", queueDiscLimitPackets);    //buffer size(queue)
  cmd.AddValue ("queueDiscType", "Set Queue disc type to RED or ARED", queueDiscType);
  cmd.AddValue ("appPktSize", "Set OnOff App Packet Size", pktSize);    //flow packet size
  cmd.AddValue ("appDataRate", "Set OnOff App DataRate", appDataRate);  //flow datarate
  cmd.AddValue ("modeBytes", "Set Queue disc mode to Packets (false) or bytes (true)", modeBytes);  //bytes or packets

  cmd.AddValue ("redMinTh", "RED queue minimum threshold", minTh);      //Qmin
  cmd.AddValue ("redMaxTh", "RED queue maximum threshold", maxTh);      //Qmax
  cmd.AddValue ("tracing", "to enable ascii and pcap tracing", tracing);
  cmd.AddValue ("nFlows", "to set the number of total flows", nFlows);
  cmd.AddValue("Anim","to enable netanim animation",animation);
  cmd.AddValue("flow_maxPackets_sent","maximum packets to send by client",flow_maxPackets_sent);
  cmd.Parse (argc,argv);

  std::string TP_dat_file = queueDiscType+"_ITP.dat";
  std::string PL_dat_file = queueDiscType+"_IPL.dat";
  out1.open(TP_dat_file);
  out2.open(PL_dat_file);

  out1<<"# X   Y"<<std::endl;
  out2<<"# X   Y"<<std::endl;

  // flow_maxPackets_sent = ceil(10*(1e6)*clientapp_simulation_time*(nFlows/2)/(pktSize*8));
  
  if(flow_maxPackets_sent != 0)
  {
    double temp = ((flow_maxPackets_sent*pktSize*8.0/(nFlows/2))/clientapp_simulation_time)/(1e6);

    std::cout<<"temp: "<<temp<<std::endl;

    appDataRate=std::to_string(temp) + "Mbps";
  }

  std::cout<<"datarate: "<<appDataRate<<std::endl;

  if ((queueDiscType != "RED") && (queueDiscType != "ARED"))
    {
      std::cout << "Invalid queue disc type: Use --queueDiscType=RED or --queueDiscType=ARED" << std::endl;
      exit (1);
    }

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (pktSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (appDataRate));

  Config::SetDefault ("ns3::DropTailQueue<Packet>::MaxSize",
                      StringValue (std::to_string (maxPackets) + "p"));

  if (!modeBytes)
    {
      Config::SetDefault ("ns3::RedQueueDisc::MaxSize",
                          QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, queueDiscLimitPackets)));
    }
  else
    {
      Config::SetDefault ("ns3::RedQueueDisc::MaxSize",
                          QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, queueDiscLimitPackets * pktSize)));
      minTh *= pktSize;
      maxTh *= pktSize;
    }

  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (minTh));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (maxTh));
  Config::SetDefault ("ns3::RedQueueDisc::LinkBandwidth", StringValue (bottleNeckLinkBw));
  Config::SetDefault ("ns3::RedQueueDisc::LinkDelay", StringValue (bottleNeckLinkDelay));
  Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (pktSize));

  if (queueDiscType == "ARED")
    {
      // Turn on ARED
      Config::SetDefault ("ns3::RedQueueDisc::ARED", BooleanValue (true));
      Config::SetDefault ("ns3::RedQueueDisc::LInterm", DoubleValue (10.0));
    }

  // Create the point-to-point link helpers
  PointToPointHelper bottleNeckLink;
  bottleNeckLink.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckLinkBw));
  bottleNeckLink.SetChannelAttribute ("Delay", StringValue (bottleNeckLinkDelay));

  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute    ("DataRate", StringValue ("10Mbps"));
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("1ms"));

  PointToPointDumbbellHelper d (nLeaf, pointToPointLeaf,
                                nLeaf, pointToPointLeaf,
                                bottleNeckLink);

  // Install Stack
  InternetStackHelper stack;
  for (uint32_t i = 0; i < d.LeftCount (); ++i)
    {
      stack.Install (d.GetLeft (i));
    }
  for (uint32_t i = 0; i < d.RightCount (); ++i)
    {
      stack.Install (d.GetRight (i));
    }

  stack.Install (d.GetLeft ());
  stack.Install (d.GetRight ());
  TrafficControlHelper tchBottleneck;
  QueueDiscContainer queueDiscs;
  QueueDiscContainer queueDiscs_left;
  tchBottleneck.SetRootQueueDisc ("ns3::RedQueueDisc");
  queueDiscs_left = tchBottleneck.Install (d.GetLeft ()->GetDevice (0));
  queueDiscs = tchBottleneck.Install (d.GetRight ()->GetDevice (0));

  // Assign IP Addresses
  d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

  //Install on/off app on all right side nodes
  OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
  clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000]"));
  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApps;
  std::cout<<d.LeftCount()<<std::endl;
  for (uint32_t i = 0; i < d.LeftCount (); ++i)
    {
      sinkApps.Add (packetSinkHelper.Install (d.GetLeft (i)));
    }
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (30.0));

  ApplicationContainer clientApps;
  uint32_t cnt=0;
  for (uint32_t i = 0; i < d.RightCount (); ++i)
    {
      // Create an on/off app sending packets to the left side
      AddressValue remoteAddress (InetSocketAddress (d.GetLeftIpv4Address (i), port));
      clientHelper.SetAttribute ("Remote", remoteAddress);
      clientApps.Add (clientHelper.Install (d.GetRight (i)));

      cnt++;

      if(cnt >= nFlows/2 && nFlows != 0)
      {
        break;
      }
    }
  

  // bool ok = true;
  // for(uint32_t i=0;i<d.RightCount();i++)
  // {
  //   for(uint32_t j=0;j<d.LeftCount();j++)
  //   {

  //     // Create an on/off app sending packets to the left side
  //     AddressValue remoteAddress (InetSocketAddress (d.GetLeftIpv4Address (j), port));
  //     clientHelper.SetAttribute ("Remote", remoteAddress);
  //     clientApps.Add (clientHelper.Install (d.GetRight (i)));

  //     cnt++;
  //     if(cnt >= nFlows/2)
  //     {
  //       ok = false;
  //       break;
  //     }

  //   }

  //   if(!ok)
  //   {
  //     break;
  //   }

  // }

    //Alternate UDP application

  // int num_half_flows = flows/2;
  // for(int i = 0; i < num_half_flows; i++) {
  //   UdpEchoServerHelper echoServer (9);

  //   ApplicationContainer serverApps = echoServer.Install (d.GetLeft(i));   // dynamically choose a server node
  //   serverApps.Start (Seconds (1.0));
  //   serverApps.Stop (Seconds (30.0));

  //   UdpEchoClientHelper echoClient (InetSocketAddress (d.GetLeftIpv4Address (i)), port);         // telling client about server app's (ip, port)
  //   echoClient.SetAttribute ("MaxPackets", UintegerValue (maxPackets));
  //   echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.01)));
  //   echoClient.SetAttribute ("PacketSize", UintegerValue (pktSize));

  //   ApplicationContainer clientApps = echoClient.Install (d.GetRight(i));  // dynamically choose a client node
  //   clientApps.Start (Seconds (2.0));
  //   clientApps.Stop (Seconds (20.0));
  // }

  clientApps.Start (Seconds (1.0)); // Start 1 second after sink
  clientApps.Stop (Seconds (15.0)); // Stop before the sink




  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  std::cout << "Running the simulation" << std::endl;


  if(tracing)
  {
  //tracing
  AsciiTraceHelper ascii;
  bottleNeckLink.EnableAsciiAll (ascii.CreateFileStream ("bottleneck_routers.tr"));

  //pcap
  bottleNeckLink.EnablePcapAll ("bottleneck_routers");
  }

  // Ptr<FlowMonitor> flowMonitor;
  // FlowMonitorHelper flowHelper;
  // flowMonitor = flowHelper.InstallAll();
  //flowMonitor = flowHelper.GetMonitor();
  monitor=flowmon.InstallAll();



  

  if(animation)
  {
    AnimationInterface anim ("red_anime.xml");
    // anim.SetConstantPosition(d.GetRight(),1.0,2.0);
    // anim.SetConstantPosition(d.GetLeft(),3.0,4.0);
  }

  double TotalTime = 40.0;
  Simulator::Stop (Seconds (TotalTime));
  Simulator::Schedule(Seconds(1), &throughput);
  Simulator::Run ();

  


  QueueDisc::Stats st = queueDiscs.Get (0)->GetStats ();
  QueueDisc::Stats st1 = queueDiscs_left.Get (0)->GetStats ();

  if (st.GetNDroppedPackets (RedQueueDisc::UNFORCED_DROP) == 0)
    {
      std::cout << "There should be some unforced drops" << std::endl;
      exit (1);
    }

  if (st.GetNDroppedPackets (QueueDisc::INTERNAL_QUEUE_DROP) != 0)
    {
      std::cout << "There should be zero drops due to queue full" << std::endl;
      exit (1);
    }

  std::cout << "*** Stats from the bottleneck queue(right) disc ***" << std::endl;
  std::cout << st << std::endl;
  std::cout << "*** Stats from the bottleneck queue(left) disc ***" << std::endl;
  std::cout << st1 << std::endl;
  std::cout << "Destroying the simulation" << std::endl;

  monitor->SerializeToXmlFile("red_flow.xml", false, false);

  Simulator::Destroy ();
  return 0;
}


