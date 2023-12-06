
<!DOCTYPE html>
<html>
<head>
    <title>Insert Data into MongoDB</title>
</head>
<body>

<?php
//composer require mongodb/mongodb

// Check if the form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Collect data from the textbox
    $data = $_POST["textbox_data"];

    // Validate data (you can add more validation as needed)

    // Connect to MongoDB
    $mongoClient = new MongoDB\Client("mongodb://localhost:27017");

    // Select the database and collection
    $database = $mongoClient->your_database; // Replace with your actual database name
    $collection = $database->your_collection; // Replace with your actual collection name

    // Insert data into MongoDB
    $document = [
        "field_name" => $data, // Replace with your actual field name
    ];

    $result = $collection->insertOne($document);

    // Check if the insertion was successful
    if ($result->getInsertedCount() > 0) {
        echo "Data successfully inserted into MongoDB!";
    } else {
        echo "Failed to insert data into MongoDB.";
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
