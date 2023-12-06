

<!DOCTYPE html>
<html>
<head>
    <title>Insert Data into Apache Cassandra</title>
</head>
<body>

<?php
//composer require datastax/php-driver

// Check if the form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Collect data from the textbox
    $data = $_POST["textbox_data"];

    // Validate data (you can add more validation as needed)

    // Connect to Cassandra
    $cluster = Cassandra::cluster()
        ->withContactPoints('127.0.0.1') // Replace with your Cassandra nodes
        ->withPort(9042)
        ->build();

    $session = $cluster->connect('your_keyspace'); // Replace with your actual keyspace

    // Insert data into Cassandra
    $statement = $session->prepare("INSERT INTO your_table (column_name) VALUES (?)"); // Replace with your actual table and column name

    $boundStatement = $statement->bind([$data]);

    $result = $session->execute($boundStatement);

    // Check if the insertion was successful
    if ($result) {
        echo "Data successfully inserted into Cassandra!";
    } else {
        echo "Failed to insert data into Cassandra.";
    }
}
?>

<!-- HTML form with a textbox -->
<form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]); ?>">
    Enter Data: <input type="text" name="textbox_data">
    <input type="submit" value="Submit">
</form>

</body>
</html>
