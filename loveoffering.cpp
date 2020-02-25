#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

class loveoffering : public eosio::contract {
   public:
      loveoffering(account_name self):eosio::contract(self), _projects(self,self),_donations(self,self)
      {}
      
      /// @abi action
      void version()
      {
          print("LoveOffering version 0.1"); 
      };
      
      
      /// @abi action
      void addproject(std::string projectName, std::string projectIntro)
      {
         require_auth( _self );

            print("Add project: ", projectName); 
            print("Project Introduction: ", projectIntro); 

            _projects.emplace(get_self(),[&](auto& p)
                                          {
                                             p.key = _projects.available_primary_key();
                                             p.projectId = _projects.available_primary_key();
                                             p.projectName = projectName;
                                             p.projectStatus = 1;
                                             p.intro = projectIntro;
                                             p.quantityWanted = projectToken;
                                          });
      };


      /// @abi action
      void offerlove(account_name donator, std::string projectName, std::string accountName)
      {
         print("Donate for ", projectName, tokenQuantity, " by ", accountName);

         struct transfer_args
         {
            account_name from;
            account_name to;
            asset quantity;
            std::string memo;
         };

         action(permission_level{donator, N(active)},
           N(eosio.token), N(transfer),
           transfer_args{
               donator,
               _self,
                asset(100, S(4, SYS)),
               "Sent to the loveoffering contract!"}
          ).send();

         uint64_t projectId =99999; // get the pollId for the _votes table

          // find the poll and the option and increment the count
          for(auto& item : _projects)
          {
              if (item.projectName == projectName)
              {
                  projectId = item.projectId; // for recording vote in this poll

                  _projects.modify(item, get_self(), [&](auto& p)
                                                {
                                                    p.quantityReceived = p.quantityReceived + 1;
                                                });
              }
          }

                   // record that accountName has voted
          _donations.emplace(get_self(), [&](auto& pv)
                                      {
                                        pv.key = _donations.available_primary_key();
                                        pv.projectId = projectId;
                                        pv.projectName = projectName;
                                        //pv.account = accountName;
                                      });             
      }

   
   



  private:    

    // create the multi index tables to store the data

      /// @abi table
      struct project 
      {
        uint64_t key; // primary key
        uint64_t projectId;
        account_name projectOwner;
        std::string projectName; // name of project
        uint8_t projectStatus = 0; // staus where 0 = closed, 1 = open, 2 = finished
        std::string   intro;//IPFS Resource
        uint64_t quantityWanted;
        uint64_t quantityReceived = 0;

        uint64_t primary_key() const { return key; }
        uint64_t by_projectId() const {return projectId; }
      };
      typedef eosio::multi_index<N(project), project, indexed_by<N(projectId), const_mem_fun<project, uint64_t, &project::by_projectId>>> projectstable;


      /// @abi table
      struct donation 
      {
        uint64_t     key; 
        uint64_t projectId;
        std::string projectName;
        account_name  account;
        //uint64_t quantityDonated;
        std::string thanksTokenAddress;

        uint64_t primary_key() const { return key; }
        uint64_t by_projectId() const {return projectId; }
      };
      typedef eosio::multi_index<N(donation), donation, indexed_by<N(projectId), const_mem_fun<donation, uint64_t, &donation::projectId>>> donations;

      // local instances of the multi indexes
      projectstable _projects;
      donations _donations;
};
