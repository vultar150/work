#pragma once

typedef enum action_type_e {
    ACTION_NONE = -1,             ///< No action (useful for testing)
    ACTION_OUTPUT,                ///< Send packet to specified port
    ACTION_DROP,                  ///< Drop packet
    ACTION_PUSH_VLAN,             ///< Add space for 802.1Q header
    ACTION_POP_VLAN,              ///< Remove 802.1Q header
    ACTION_SET_VLAN,              ///< Change 802.1Q VID field value
    ACTION_GROUP,                 ///< Process header in specified group
    ACTION_SET_DST_MAC,           ///< Change destination MAC address field value
    ACTION_SET_DST_MAC_FROM_META, /*!< Change destination MAC address field value. The new
                                       value is loaded from OpenFlow metadata */
    ACTION_SET_DST_MAC_FROM_ACC,  /*!< Change destination MAC address field value. The new
                                       value is loaded from accumulator */
    ACTION_SET_SRC_MAC,           ///< Change source MAC address field value
    ACTION_SET_SRC_MAC_FROM_META, /*!< Change source MAC address field value. The new
                                       value is loaded from OpenFlow metadata */
    ACTION_SET_SRC_MAC_FROM_ACC,  /*!< Change source MAC address field value. The new
                                       value is loaded from accumulator */
    ACTION_DEC_IPV4_TTL,          ///< Decrement TTL field in IPv4 header
    ACTION_DEC_MPLS_TTL,          ///< Decrement TTL field in MPLS header
    ACTION_COPY_IPV4_MPLS_TTL,    ///< Copy TTL from IPv4 header to MPLS header
    ACTION_COPY_MPLS_IPV4_TTL,    ///< Copy TTL from MPLS header to IPv4 header
    ACTION_COPY_MPLS_TTL_OUT,     /*!< Copy TTL from the next-to-outermost MPLS header to
                                       the outermost MPLS header */
    ACTION_COPY_MPLS_TTL_IN,      /*!< Copy TTL from the outermost MPLS header to the
                                       next-to-outermost MPLS header */
    ACTION_PUSH_MPLS,             ///< Add space for MPLS header
    ACTION_POP_MPLS,              ///< Remove MPLS header
    ACTION_SET_MPLS,              ///< Change MPLS label field value
    ACTION_SET_MPLS_FROM_META,    /*!< Change MPLS label field value. The new value is
                                       loaded from OpenFlow metadata */
    ACTION_SET_FLAG,              ///< Change Flag metadata field value
    INSTR_GOTO_TABLE,             ///< Resume classification in specified table
    INSTR_WRITE_METADATA,         /*!< Write a metadata value into the OpenFlow metadata
                                       field */
} action_type_t;

typedef uint64_t action_value_t;

typedef struct instr_s {
    action_type_t type;
    action_value_t value;
} instr_t;

