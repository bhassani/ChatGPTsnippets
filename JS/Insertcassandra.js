const express = require('express');
const cassandra = require('cassandra-driver');
const bodyParser = require('body-parser');

const app = express();
const port = 3000;

// Cassandra connection
const client = new cassandra.Client({
  contactPoints: ['127.0.0.1'], // Replace with your Cassandra nodes
  localDataCenter: 'datacenter1',
  keyspace: 'your_keyspace', // Replace with your actual keyspace
});

// Body parser middleware
app.use(bodyParser.urlencoded({ extended: true }));

// Serve HTML form
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});

// Handle form submission
app.post('/submit', (req, res) => {
  const data = req.body.textbox_data;

  // Insert data into Cassandra
  const query = 'INSERT INTO your_table (column_name) VALUES (?)'; // Replace with your actual table and column name

  client.execute(query, [data], { prepare: true }, (err, result) => {
    if (err) {
      console.error('Error inserting data:', err);
      res.send('Failed to insert data into Cassandra.');
    } else {
      console.log('Data successfully inserted into Cassandra!');
      res.send('Data successfully inserted into Cassandra!');
    }
  });
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
