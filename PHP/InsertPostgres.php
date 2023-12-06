<!DOCTYPE html>
<html>
<head>
    <title>Insert Data into PostgreSQL</title>
</head>
<body>

<?php
// Check if the form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Collect data from the textbox
    $data = $_POST["textbox_data"];

    // Validate data (you can add more validation as needed)

    // Connect to PostgreSQL database
    $db_connection = pg_connect("host=localhost dbname=your_database user=your_username password=your_password");

    // Check if the connection is successful
    if (!$db_connection) {
        die("Connection failed: " . pg_last_error());
    }

    // Insert data into the database
    $query = "INSERT INTO your_table (column_name) VALUES ('$data')";

    $result = pg_query($db_connection, $query);

    // Check if the query was successful
    if (!$result) {
        die("Query failed: " . pg_last_error());
    }

    // Close the database connection
    pg_close($db_connection);

    echo "Data successfully inserted into PostgreSQL database!";
}
?>

<!-- HTML form with a textbox -->
<form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]); ?>">
    Enter Data: <input type="text" name="textbox_data">
    <input type="submit" value="Submit">
</form>

</body>
</html>
