<?php

function calculateExpectedScore($ratingA, $ratingB) {
    return 1 / (1 + pow(10, ($ratingB - $ratingA) / 400));
}

function updateRating($rating, $expectedScore, $actualScore, $k = 32) {
    return $rating + $k * ($actualScore - $expectedScore);
}

function eloRatingWinnerLoser($winnerRating, $loserRating, $k = 32) {
    $expectedWinner = calculateExpectedScore($winnerRating, $loserRating);
    $expectedLoser = calculateExpectedScore($loserRating, $winnerRating);

    $updatedWinner = updateRating($winnerRating, $expectedWinner, 1, $k);
    $updatedLoser = updateRating($loserRating, $expectedLoser, 0, $k);

    return [$updatedWinner, $updatedLoser];
}

// Example usage:
$playerARating = 1600;
$playerBRating = 1500;

[$newRatingA, $newRatingB] = eloRatingWinnerLoser($playerARating, $playerBRating);

echo "New ratings after the match:\n";
echo "Player A: $newRatingA\n";
echo "Player B: $newRatingB\n";

?>
