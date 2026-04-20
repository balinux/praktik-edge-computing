import mqtt from "mqtt";
import axios from "axios";

import config from "./edge/config.js";
import rules from "./edge/rules.js";

const client = mqtt.connect(config.mqttBroker);

client.on("connect", () => {

    console.log("Edge Connected");

    client.subscribe(config.topics.suhu);

});

client.on("message", async (topic, message) => {

    const data = JSON.parse(message.toString());

    console.log("Receive:", data);

    // ======================
    // ANALYZE
    // ======================

    const action = rules(data);

    console.log("Analyze:", action);

    // ======================
    // EXECUTE
    // ======================

    if (action.kipas) {

        client.publish(config.topics.kipas, action.kipas);
        console.log("Execute kipas:", action.kipas);

    }

    if (action.lampu) {

        client.publish(config.topics.lampu, action.lampu);
        console.log("Execute lampu:", action.lampu);

    }

    // ======================
    // SEND TO CLOUD
    // ======================

    // const url = config.thingsboard.url + config.thingsboard.token + "/telemetry";

    // await axios.post(url, data);

    // console.log("Send to cloud");

    const url = `http://demo.thingsboard.io/api/v1/${config.thingsboard.token}/telemetry`;

await axios.post(url, {
    temperature: data.temperature,
    kipas: action.kipas || "OFF",
    lampu: action.lampu || "OFF"
}, {
    headers: {
        "Content-Type": "application/json"
    }
});

});