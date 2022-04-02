#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

void testWrongBind()
{
	int inet6Socket = socket(AF_INET6, SOCK_STREAM, getprotobyname("tcp")->p_proto);
	{
		struct sockaddr_in sa;
		sa.sin_family = AF_INET;
		sa.sin_port = htons(13123);
		sa.sin_addr.s_addr = 0;
		if (bind(inet6Socket, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			perror("Expected, failed to bind inet4 to inet6 socket");
		}
	}
	{
		struct sockaddr_in6 sa;
		sa.sin6_family = AF_INET6;
		sa.sin6_port = htons(13123);
		inet_pton(AF_INET6, "::/128", &sa.sin6_addr);
		if (bind(inet6Socket, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			perror("Failed to bind inet6 to inet6 socket");
		}
	}
	close(inet6Socket);
}

void testAllowedBind()
{
	int inet6Socket = socket(AF_INET6, SOCK_STREAM, getprotobyname("tcp")->p_proto);
	int value = 0;
	if (setsockopt(inet6Socket, SOL_IPV6, IPV6_V6ONLY, &value, sizeof(value)) < 0) {
		perror("setsockopt to IPV6_V6ONLY=0 failed");
	}
	{
		struct sockaddr_in sa;
		sa.sin_family = AF_INET;
		sa.sin_port = htons(13123);
		sa.sin_addr.s_addr = 0;
		if (bind(inet6Socket, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			perror("Failed to bind inet4 to inet6 socket");
		}
	}
	{
		struct sockaddr_in6 sa;
		sa.sin6_family = AF_INET6;
		sa.sin6_port = htons(13123);
		inet_pton(AF_INET6, "::/128", &sa.sin6_addr);
		if (bind(inet6Socket, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			perror("Failed to bind inet6 to inet6 socket");
		}
	}
	close(inet6Socket);
}

int main(void)
{
	testWrongBind();
	testAllowedBind();
	return 0;
}
