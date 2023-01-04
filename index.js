require('dotenv').config()
// console.log(process.env)
const express = require('express')
const db = require("./models")
const app = express()
app.use(express.json())
const port = process.env.PORT || 3000
const { check, validationResult } = require('express-validator')
const mqtt = require('mqtt')

db.mongoose.connect(
    `mongodb://${process.env.MONGODB_USERNAME}:${process.env.MONGODB_PASSWORD}@${process.env.MONGODB_HOST}:${process.env.MONGODB_PORT}/${process.env.DB_NAME}`
    ).then(() => {
        console.log("Connection to database Success")
    })
    .catch(err => {
        console.log("Connection error ", err)
        process.exit();
    }
)

app.get('/', (req, res) => {
    res.send('Hello World!')
})

app.get('/getData/:deviceId', (req, res) => {
    db.dev_result.find({'deviceId':req.params.deviceId},function (err, docs) {
        // console.log(docs)
        res.json(docs)
    })
})

app.get('/getData', (req, res) => {
    db.dev_result.find(function (err, docs) {
        // console.log(docs)
        res.json(docs)
    })
})

app.post('/insertData',
    check('deviceId','deviceId is required').not().isEmpty(),
    async (req, res) => {
        const errors = validationResult(req)
        if (!errors.isEmpty()) {
            return res.status(400).json({ errors: errors.array() })
        }
        console.log(req.body)

        try {
            const result = await db.dev_result.create({
                deviceId: req.body.deviceId,
                moisture: req.body.moisture,
                ph: req.body.ph,
                relay_water: req.body.relay_water,
                relay_acid: req.body.relay_acid,
                relay_base: req.body.relay_base
            })
            // console.log(result)
            res.json(result)
        }
        catch (error) {
            // console.log(error)
            res.json(error)
        }
})

app.listen(port, () => {
    console.log(`Example app listening on port ${port}`)
})

const client  = mqtt.connect(
    `mqtt://${process.env.MQTT_HOST}:${process.env.MQTT_PORT}`,
    {
        "username":process.env.MQTT_USERNAME,
        "password":process.env.MQTT_PASSWORD
    }
)

client.on('connect', function () {
    client.subscribe(`${process.env.MQTT_ROOT_TOPIC}/moisture`)
    client.subscribe(`${process.env.MQTT_ROOT_TOPIC}/ph`)
    client.subscribe(`${process.env.MQTT_ROOT_TOPIC}/relay_water`)
    client.subscribe(`${process.env.MQTT_ROOT_TOPIC}/relay_acid`)
    client.subscribe(`${process.env.MQTT_ROOT_TOPIC}/relay_base`)
})

client.on('message', function (topic, message) {
    // message is Buffer
    console.log(topic)
    console.log(message.toString())
})

