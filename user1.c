/* THE SERVER PROCESS */
#include <time.h>
#include "mysocket.h"

int main(){
    int                 sockfd, newsockfd;                                  // Socket descriptors
    unsigned int                 clilen;
    struct sockaddr_in  cli_addr, serv_addr;
    char                buff[5000];                                          // We will use this buffer for communication              

    if((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0))<0){                       // The following system call opens a socket
        perror("Cannot create socket\n");
        exit(0);
    }
    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_addr.s_addr   = INADDR_ANY;
    serv_addr.sin_port          = htons(22461);

    if(my_bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0){  // With the information provided in serv_addr, we associate the server
	                                                                        // with its port using the bind() system call. 
        perror("Unable to bind local address\n");
        exit(0);
    }

    my_listen(sockfd, 5);                                                      // atmost 5 client can be queued
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = my_accept(sockfd, (struct sockaddr *) &cli_addr,           // The accept() system call accepts a client connection.
					&clilen) ;
        if(newsockfd < 0){
            perror("Accept error\n");
            exit(0);
        }
        time_t rawtime;                                                     // following code takes time from the system
        struct tm * timeinfo;

        
        strcpy(buff, "Krishna, Sanskrit Kṛṣṇa, one of the most widely revered and most popular of all Indian divinities, worshipped as the eighth incarnation (avatar, or avatara) of the Hindu god Vishnu and also as a supreme god in his own right. Krishna became the focus of numerous bhakti (devotional) cults, which have over the centuries produced a wealth of religious poetry, music, and painting. The basic sources of Krishna’s mythology are the epic Mahabharata and its 5th-century-CE appendix, the Harivamsha, and the Puranas, particularly Books X and XI of the Bhagavata-purana. They relate how Krishna (literally “black,” or “dark as a cloud”) was born into the Yadava clan, the son of Vasudeva and Devaki, who was the sister of Kamsa, the wicked king of Mathura (in modern Uttar Pradesh). Kamsa, hearing a prophecy that he would be destroyed by Devaki’s child, tried to slay her children, but Krishna was smuggled across the Yamuna River to Gokula (or Vraja, modern Gokul), where he was raised by the leader of the cowherds, Nanda, and his wife Yashoda.The child Krishna was adored for his mischievous pranks; he also performed many miracles and slew demons. As a youth, the cowherd Krishna became renowned as a lover, the sound of his flute prompting the gopis (wives and daughters of the cowherds) to leave their homes to dance ecstatically with him in the moonlight. His favourite among them was the beautiful Radha. At length, Krishna and his brother Balarama returned to Mathura to slay the wicked Kamsa. Afterward, finding the kingdom unsafe, Krishna led the Yadavas to the western coast of Kathiawar and established his court at Dvaraka (modern Dwarka, Gujarat). He married the princess Rukmini and took other wives as well.Krishna refused to bear arms in the great war between the Kauravas (sons of Dhritarashtra, the descendant of Kuru) and the Pandavas (sons of Pandu), but he offered a choice of his personal attendance to one side and the loan of his army to the other. The Pandavas chose the former, and Krishna thus served as charioteer for Arjuna, one of the Pandava brothers. On his return to Dvaraka, a brawl broke out one day among the Yadava chiefs in which Krishna’s brother and son were slain. As the god sat in the forest lamenting, a huntsman, mistaking him for a deer, shot him in his one vulnerable spot, the heel, killing him.Krishna’s personality is clearly a composite one, though the different elements are not easily separated. Vasudeva-Krishna was deified by the 5th century BCE. The cowherd Krishna was probably the god of a pastoral community. The Krishna who emerged from the blending of these figures was ultimately identified with the supreme god Vishnu-Narayana and, hence, considered his avatar. His worship preserved distinctive traits, chief among them an exploration of the analogies between divine love and human love. Thus, Krishna’s youthful dalliances with the gopis are interpreted as symbolic of the loving interplay between God and the human soul.The rich variety of legends associated with Krishna’s life led to an abundance of representation in painting and sculpture. The child Krishna (Balakrishna) is depicted crawling on his hands and knees or dancing with joy, a ball of butter held in his hands. The divine lover—the most common representation—is shown playing the flute, surrounded by adoring gopis. In 17th- and 18th-century Rajasthani and Pahari painting, Krishna is characteristically depicted with blue-black skin, wearing a yellow dhoti (loincloth) and a crown of peacock feathers.Purana, (Sanskrit: “Ancient”) in the sacred literature of Hinduism, any of a number of popular encyclopaedic collections of myth, legend, and genealogy, varying greatly as to date and origin.Puranas were written almost entirely in narrative couplets, in much the same easy flowing style as the two great Sanskrit epic poems, the Mahabharata and the Ramayana. The early Puranas were probably compiled by upper-caste authors who appropriated popular beliefs and ideas from people of various castes. Later Puranas reveal evidence of vernacular influences and the infusion of local religious traditions.");
        printf("sent message of %ld length\n",strlen(buff));
        my_send(newsockfd, buff,strlen(buff),0);                             // sending the time and date to the client
        close(newsockfd);

    }
    return 0;
}