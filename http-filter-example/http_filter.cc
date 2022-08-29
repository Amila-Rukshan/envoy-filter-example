#include <string>

#include "http_filter.h"

#include "envoy/server/filter_config.h"

#include "source/common/network/filter_state_proxy_info.h"
#include "source/common/network/utility.h"
#include "source/extensions/filters/http/common/pass_through_filter.h"

namespace Envoy {
namespace Http {

HttpSampleDecoderFilterConfig::HttpSampleDecoderFilterConfig(
    const sample::Decoder& proto_config)
    : key_(proto_config.key()), val_(proto_config.val()) {}

HttpSampleDecoderFilter::HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr config)
    : config_(config) {}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

const LowerCaseString HttpSampleDecoderFilter::headerKey() const {
  return LowerCaseString(config_->key());
}

const std::string HttpSampleDecoderFilter::headerValue() const {
  return config_->val();
}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& request_headers, bool) {
  // add a header
  request_headers.addCopy(headerKey(), headerValue());
  auto connect_proxy = LowerCaseString("connect-proxy");
  auto hostname = request_headers.getHostValue();
  ASSERT(!hostname.empty());
  if (!request_headers.get(connect_proxy).empty()) {
    std::string address_string(request_headers.get(connect_proxy)[0]->value().getStringView());
    auto address = Network::Utility::parseInternetAddressAndPort(address_string);
    decoder_callbacks_->streamInfo().filterState()->setData(
        Network::Http11ProxyInfoFilterState::key(),
        std::make_unique<Network::Http11ProxyInfoFilterState>(hostname, address),
        StreamInfo::FilterState::StateType::ReadOnly,
        StreamInfo::FilterState::LifeSpan::FilterChain);
    request_headers.remove(connect_proxy);
  }
  return FilterHeadersStatus::Continue;
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

void HttpSampleDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

} // namespace Http
} // namespace Envoy
