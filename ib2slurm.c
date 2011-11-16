#include "ib2slurm.h"

nn_map_t *node_name_map = NULL;

void switch_iter_func(ibnd_node_t * node, void *curry)
{
    ibnd_port_t *port;
    int p = 0;

    char * switchname = node_name(node);
    fprintf(stdout, "SwitchName=%s", switchname);
    free(switchname);

    for (p = 1; p <= node->numports; p++) {
        fprintf(stdout, " Switches=");

        if(node->type == IB_NODE_SWITCH) {
            port = node->ports[p];
            if (port && port->remoteport) {
                char *remotename = node_name(port->remoteport);
                fprintf(stdout, "%s,", remotename);
                free(remotename);
            }
        }
    }

    for (p = 1; p <= node->numports; p++) {
        fprintf(stdout, " Nodes=");

        if(node->type == IB_NODE_CA) {
            port = node->ports[p];
            if (port && port->remoteport) {
                char *remotename = node_name(port->remoteport);
                fprintf(stdout, "%s,", remotename);
                free(remotename);
            }
        }
    }

    fprintf(stdout, "\n");
}

char *node_name(ibnd_node_t * node)
{
    /* FIXME: does this always output something sane? */
    return remap_node_name(node_name_map, node->guid, node->nodedesc);
}

int main(int argc, char** argv)
{
    struct ibnd_config config = {0};
    ibnd_fabric_t *fabric = NULL;
    char *node_name_map_file = NULL;

    char *ibd_ca = NULL;
    int ibd_ca_port = 0;

    int option_index = 0;
    int c;
    static struct option long_options[] = {
        {"node-name-map", 1, 0, 'm'},
        {NULL, 0, NULL, 0}
    };

    while((c = getopt_long(argc, argv, "m:", long_options, &option_index)) != -1) {
        switch(c) {
            case 'm':
                node_name_map_file = strdup(optarg);
                break;
        }
    }

    node_name_map = open_node_name_map(node_name_map_file);

    if ((fabric = ibnd_discover_fabric(ibd_ca, ibd_ca_port, NULL, &config)) == NULL) {
        fprintf(stderr, "IB discover failed.\n");
        exit(EXIT_FAILURE);
    }

    ibnd_iter_nodes_type(fabric, switch_iter_func, IB_NODE_SWITCH, NULL);

    ibnd_destroy_fabric(fabric);
    close_node_name_map(node_name_map);

    exit(EXIT_SUCCESS);
}

/* EOF */
