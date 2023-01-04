const mongoose = require("mongoose");

const devResult = new mongoose.Schema({
        deviceId: {type: String, default: null},
        moisture: {type: Number},
        ph: {type: Number},
        relay_water :{type: Number},
        relay_acid :{type: Number},
        relay_base :{type: Number}
    },
    { timestamps: true }
);

module.exports = mongoose.model("dev_result", devResult)