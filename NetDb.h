#ifndef NETDB_H__
#define NETDB_H__

#include <inttypes.h>
#include <set>
#include <map>
#include <list>
#include <string>
#include <thread>
#include <mutex>
#include <boost/filesystem.hpp>
#include "Queue.h"
#include "I2NPProtocol.h"
#include "RouterInfo.h"
#include "LeaseSet.h"
#include "Tunnel.h"
#include "TunnelPool.h"
#include "Reseed.h"
#include "NetDbRequests.h"

namespace i2p
{
namespace data
{		
	
	class NetDb
	{
		public:

			NetDb ();
			~NetDb ();

			void Start ();
			void Stop ();
			
			void AddRouterInfo (const uint8_t * buf, int len);
			void AddRouterInfo (const IdentHash& ident, const uint8_t * buf, int len);
			void AddLeaseSet (const IdentHash& ident, const uint8_t * buf, int len, std::shared_ptr<i2p::tunnel::InboundTunnel> from);
			std::shared_ptr<RouterInfo> FindRouter (const IdentHash& ident) const;
			std::shared_ptr<LeaseSet> FindLeaseSet (const IdentHash& destination) const;

			void RequestDestination (const IdentHash& destination, RequestedDestination::RequestComplete requestComplete = nullptr);			
			
			void HandleDatabaseStoreMsg (I2NPMessage * msg);
			void HandleDatabaseSearchReplyMsg (I2NPMessage * msg);
			void HandleDatabaseLookupMsg (I2NPMessage * msg);			

			std::shared_ptr<const RouterInfo> GetRandomRouter () const;
			std::shared_ptr<const RouterInfo> GetRandomRouter (std::shared_ptr<const RouterInfo> compatibleWith) const;
			std::shared_ptr<const RouterInfo> GetHighBandwidthRandomRouter (std::shared_ptr<const RouterInfo> compatibleWith) const;
			std::shared_ptr<const RouterInfo> GetRandomPeerTestRouter () const;
			std::shared_ptr<const RouterInfo> GetRandomIntroducer () const;
			std::shared_ptr<const RouterInfo> GetClosestFloodfill (const IdentHash& destination, const std::set<IdentHash>& excluded) const;
			std::shared_ptr<const RouterInfo> GetClosestNonFloodfill (const IdentHash& destination, const std::set<IdentHash>& excluded) const;
			void SetUnreachable (const IdentHash& ident, bool unreachable);			

			void PostI2NPMsg (I2NPMessage * msg);

			void Reseed ();

			// for web interface
			int GetNumRouters () const { return m_RouterInfos.size (); };
			int GetNumFloodfills () const { return m_Floodfills.size (); };
			int GetNumLeaseSets () const { return m_LeaseSets.size (); };
			
		private:

			bool CreateNetDb(boost::filesystem::path directory);
			void Load (const char * directory);
			void SaveUpdated (const char * directory);
			void Run (); // exploratory thread
			void Explore (int numDestinations);	
			void Publish ();
			void ManageLeaseSets ();
			void ManageRequests ();

			template<typename Filter>
			std::shared_ptr<const RouterInfo> GetRandomRouter (Filter filter) const;	
		
		private:

			std::map<IdentHash, std::shared_ptr<LeaseSet> > m_LeaseSets;
			mutable std::mutex m_RouterInfosMutex;
			std::map<IdentHash, std::shared_ptr<RouterInfo> > m_RouterInfos;
			mutable std::mutex m_FloodfillsMutex;
			std::list<std::shared_ptr<RouterInfo> > m_Floodfills;
			
			bool m_IsRunning;
			std::thread * m_Thread;	
			i2p::util::Queue<I2NPMessage> m_Queue; // of I2NPDatabaseStoreMsg

			Reseeder * m_Reseeder;

			friend NetDbRequests; 
			NetDbRequests m_Requests;

			static const char m_NetDbPath[];
	};

	extern NetDb netdb;
}
}

#endif
