#ifndef _GPD_XS_MAPPER_INCLUDED
#define _GPD_XS_MAPPER_INCLUDED

#undef New

#include <upb/pb/encoder.h>
#include <upb/pb/decoder.h>
#include <upb/bindings/stdc++/string.h>

#include "EXTERN.h"
#include "perl.h"

namespace gpd {

class Dynamic;

class Mapper {
public:
    struct Field {
        const upb::FieldDef *field_def;
        struct {
            upb_selector_t seq_start;
            upb_selector_t seq_end;
            union {
                struct {
                    upb_selector_t msg_start;
                    upb_selector_t msg_end;
                };
                struct {
                    upb_selector_t str_start;
                    upb_selector_t str_cont;
                    upb_selector_t str_end;
                };
                upb_selector_t primitive;
            };
        } selector;
        SV *name;
        U32 name_hash;
        bool has_default;
        const Mapper *mapper; // for Message/Group fields
    };

    struct DecoderHandlers {
        std::vector<SV *> items;
        std::vector<const Mapper *> mappers;
        std::vector<std::vector<bool> > seen_fields;
        SV *string;

        DecoderHandlers(const Mapper *mapper);

        void prepare(HV *target);
        SV *get_target();

        static bool on_end_message(DecoderHandlers *cxt, upb::Status *status);
        static DecoderHandlers *on_start_string(DecoderHandlers *cxt, const int *field_index, size_t size_hint);
        static size_t on_string(DecoderHandlers *cxt, const int *field_index, const char *buf, size_t len);
        static bool on_end_string(DecoderHandlers *cxt, const int *field_index);
        static DecoderHandlers *on_start_sequence(DecoderHandlers *cxt, const int *field_index);
        static bool on_end_sequence(DecoderHandlers *cxt, const int *field_index);
        static DecoderHandlers *on_start_sub_message(DecoderHandlers *cxt, const int *field_index);
        static bool on_end_sub_message(DecoderHandlers *cxt, const int *field_index);

        template<class T>
        static bool on_nv(DecoderHandlers *cxt, const int *field_index, T val);

        template<class T>
        static bool on_iv(DecoderHandlers *cxt, const int *field_index, T val);

        template<class T>
        static bool on_uv(DecoderHandlers *cxt, const int *field_index, T val);

        static bool on_bool(DecoderHandlers *cxt, const int *field_index, bool val);

        void apply_defaults();
        SV *get_target(const int *field_index);
    };

public:
    Mapper(upb::reffed_ptr<const upb::MessageDef> message_def);

    void resolve_mappers(Dynamic *registry);

    DecoderHandlers *new_decoder_closure() const;

    bool encode_from_perl(SV *ref);
    bool decode_to_perl(const char *buffer, STRLEN bufsize, SV *target);

    bool encode_from_perl(upb::pb::Encoder* encoder, upb::Sink *sink, SV *ref) const;
    bool encode_from_perl(upb::pb::Encoder* encoder, upb::Sink *sink, const Field &fd, SV *ref) const;
    bool encode_from_perl_array(upb::pb::Encoder* encoder, upb::Sink *sink, const Field &fd, SV *ref) const;

public:
    upb::reffed_ptr<const upb::MessageDef> message_def;
    upb::reffed_ptr<const upb::Handlers> encoder_handlers;
    upb::reffed_ptr<upb::Handlers> decoder_handlers;
    upb::reffed_ptr<const upb::pb::DecoderMethod> decoder_method;
    std::vector<Field> fields;
    // XXX this needs to be reset when decoding/encoding (also, reentrancy)
    upb::Environment env;
    DecoderHandlers decoder_callbacks;
    upb::Sink encoder_sink, decoder_sink;
    upb::pb::Decoder *decoder;
    std::string output_buffer;
    upb::StringSink string_sink;
    upb::pb::Encoder *encoder;
};

};

#endif