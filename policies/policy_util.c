/** \file policy_util.c
 *
 *  \copyright Copyright 2013-2015 Philipp Schmidt, Theresa Enghardt, and Mirko Palmer.
 *  All rights reserved. This project is released under the New BSD License.
 */

#include "policy_util.h"

int mampol_get_socketopt(struct socketopt *list, int level, int optname, socklen_t *optlen, void *optval)
{
	struct socketopt *current = list;
	int ret = -1;

	while (current != NULL)
	{
		if (current->level == level && current->optname == optname)
		{
			if (current->optval != NULL && optval != NULL)
			{
				*optlen = current->optlen;
				memcpy(optval, current->optval, current->optlen);
			}
			ret = 0;
		}
		current = current->next;
	}
	return ret;
}

void print_pfx_addr (gpointer element, gpointer data)
{
	struct src_prefix_list *pfx = element;
	char addr_str[INET6_ADDRSTRLEN+1]; /** String for debug / error printing */

	/* Print first address of this prefix */
	if (pfx->family == AF_INET)
	{
		inet_ntop(AF_INET, &( ((struct sockaddr_in *) (pfx->if_addrs->addr))->sin_addr ), addr_str, sizeof(addr_str));
		printf("\n\t\t%s", addr_str);
	}
	else if (pfx->family == AF_INET6)
	{
		inet_ntop(AF_INET6, &( ((struct sockaddr_in6 *) (pfx->if_addrs->addr))->sin6_addr ), addr_str, sizeof(addr_str));
		printf("\n\t\t%s", addr_str);
	}

	/* Print policy info if available */
	if (pfx->policy_info != NULL)
		print_policy_info((void*) pfx->policy_info);
}

void make_v4v6_enabled_lists (GSList *baselist, GSList **v4list, GSList **v6list)
{
	printf("Configured addresses:");
	printf("\n\tAF_INET: ");
	filter_prefix_list (baselist, v4list, PFX_ENABLED, NULL, AF_INET, NULL);
	if (*v4list != NULL)
		g_slist_foreach(*v4list, &print_pfx_addr, NULL);
	else
		printf("\n\t\t(none)");

	printf("\n\tAF_INET6: ");
	filter_prefix_list (baselist, v6list, PFX_ENABLED, NULL, AF_INET6, NULL);
	if (*v6list != NULL)
		g_slist_foreach(*v6list, &print_pfx_addr, NULL);
	else
		printf("\n\t\t(none)");
}

void set_bind_sa(request_context_t *rctx, struct src_prefix_list *chosen, strbuf_t *sb)
{
	strbuf_printf(sb, "\n\tSet src=");
	_muacc_print_sockaddr(sb, chosen->if_addrs->addr, chosen->if_addrs->addr_len);

	rctx->ctx->bind_sa_suggested = _muacc_clone_sockaddr(chosen->if_addrs->addr, chosen->if_addrs->addr_len);
	rctx->ctx->bind_sa_suggested_len = chosen->if_addrs->addr_len;
}

void print_addrinfo_response (struct addrinfo *res)
{
	strbuf_t sb;
	strbuf_init(&sb);

	struct addrinfo *item = res;
	while (item != NULL)
	{
		strbuf_printf(&sb, "\t");
		if (item->ai_family == AF_INET)
			_muacc_print_sockaddr(&sb, item->ai_addr, sizeof(struct sockaddr_in));
		else if (item->ai_family == AF_INET6)
			_muacc_print_sockaddr(&sb, item->ai_addr, sizeof(struct sockaddr_in6));

		strbuf_printf(&sb, "\n");
		item = item->ai_next;
	}

	printf("%s\n", strbuf_export(&sb));
	strbuf_release(&sb);
}
