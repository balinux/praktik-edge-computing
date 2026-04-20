function analyze(data: { temperature: number }) {
  let action: { kipas?: string; lampu?: string } = {};

  if (data.temperature > 30) {
    action.kipas = "ON";
  } else {
    action.kipas = "OFF";
  }

  if (data.temperature < 25) {
    action.lampu = "ON";
  } else {
    action.lampu = "OFF";
  }

  return action;
}

export default analyze;
