const database = 'electricity';

function getRandomInt(min, max) {
    min = Math.ceil(min);
    max = Math.floor(max);
    return Math.floor(Math.random() * (max - min) + min); // The maximum is exclusive and the minimum is inclusive
}

const Influx = require('influx')
const influx = new Influx.InfluxDB({
    host: 'localhost',
    database: database, //???
    schema: [
        {
            measurement: "electricity",
            fields: {
                type: Influx.FieldType.STRING,
                value: Influx.FieldType.INTEGER
            },

            tags: [
                'device'
            ]
        }
    ]
})

influx.getDatabaseNames().then(names => {
    if (!names.includes(database)) {
        influx.createDatabase(database)
    }
});

influx.writePoints([
    {
        measurement: "electricity",
        tags: {host: 'inverter1'},
        fields: {type: "CURRENT", value: getRandomInt(10, 30)},
    }
], {
    database: database,
    precision: 's'
}).catch(function(e) {
    console.log(e);
})


