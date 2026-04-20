const mqttBroker = "mqtt://localhost:1883";
const thingsboard = {
  token: "aV4rVeDAlpgINvsb2sHp",
  url: "http://demo.thingsboard.io/api/v1/",
};
const topics = {
  suhu: "kampus/suhu",
  kipas: "kampus/kipas",
  lampu: "kampus/lampu",
};

export default {
  mqttBroker,
  thingsboard,
  topics,
};
