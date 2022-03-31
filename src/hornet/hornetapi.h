/* 
 * This file contains a string rappresentation of the IOTA REST API version: 1.0.0
 *
 * https://editor.swagger.io/?url=https://raw.githubusercontent.com/rufsam/protocol-rfcs/master/text/0026-rest-api/0026-rest-api.yaml
 * 
 * generale structure of const hornet_api_{https request}_{purpose}
 */


/* HTTPS GET
 * Returns the health of the node. A node considers itself healthy if its solid milestone is at most two
 * delta away from the latest known milestone, has at least one ongoing gossip stream and the latest known
 * milestone is newer than 5 minutes. This information might be useful for load-balancing or uptime monitoring.
 */
const char* hornet_api_get_health = "/health";

/* HTTPS GET
 * Returns general information about the node.
 */
const char* hornet_api_get_info = "/api/v1/info";

/* HTTPS POST
 * Submit a message. The node takes care of missing* fields and tries to build the message.
 * On success, the message will be stored in the Tangle. This endpoint will return the identifier
 * of the built message. *The node will try to auto-fill the following fields in case they are missing:
 * networkId, parentMessageIds, nonce. If payload is missing, the message will be built without a payload.
 */

const char* hornet_api_post_message = "/api/v1/messages";

/* HTTPS GET
 * Search for messages matching a given indexation key.
 */

const char* hornet_api_get_message = "/api/v1/messages";