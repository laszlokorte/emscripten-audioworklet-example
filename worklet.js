class WasmProcessor extends AudioWorkletProcessor {
  #memory;
  #instance;
  constructor(...args) {
    const [
      {
        processorOptions: { memory, wasm },
      },
    ] = args;
    super(...args);
    this.memory = memory;
    WebAssembly.instantiate(wasm, {
      env: { memory },
    }).then((i) => {
      this.instance = i;
    });
  }
  static get parameterDescriptors() {
    return [
      {
        name: "freq",
        defaultValue: 440,
        minValue: 1,
        maxValue: 1000,
        automationRate: "a-rate",
      },
    ];
  }
  process(inputs, outputs, parameters) {
    const blockSize = outputs[0][0].length;
    const samples = new Float32Array(this.memory.buffer, 0, blockSize);
    this.instance.exports.generate_block(
      parameters.freq[0],
      samples,
      blockSize,
    );
    const output = outputs[0];
    output.forEach((channel) => {
      for (let i = 0; i < blockSize; i++) {
        channel[i] = samples[i];
      }
    });
    return true;
  }
}

registerProcessor("wasm-processor", WasmProcessor);
